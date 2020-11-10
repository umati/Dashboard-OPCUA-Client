#include "DashboardMachineObserver.hpp"
#include <easylogging++.h>
#include "Exceptions/MachineInvalidException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <Base64.hpp>
#include <utility>

namespace Umati {
	namespace MachineObserver {

		DashboardMachineObserver::DashboardMachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
				std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher
		) : MachineObserver(std::move(pDataClient)),
			m_pPublisher(std::move(pPublisher)) {
			startUpdateMachineThread();
		}

		DashboardMachineObserver::~DashboardMachineObserver() {
			stopMachineUpdateThread();
		}

		void DashboardMachineObserver::PublishAll() {
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			for (const auto &pDashClient : m_dashboardClients) {
				pDashClient.second->Publish();
			}

			if (m_publishMachinesOnline >= 30) {
				this->publishMachinesList();
				m_publishMachinesOnline = 0;
			} else {
				m_publishMachinesOnline = m_publishMachinesOnline + 1;
			}
		}

		void DashboardMachineObserver::startUpdateMachineThread() {
			if (m_running) {
				LOG(INFO) << "Machine update thread already running";
				return;
			}

			auto func = [this]() {
				int cnt = 0;
				while (this->m_running) {
					if ((cnt % 10) == 0) {
						this->UpdateMachines();
					}

					++cnt;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			};
			m_running = true;
			m_updateMachineThread = std::thread(func);
		}

		void DashboardMachineObserver::stopMachineUpdateThread() {
			m_running = false;
			if (m_updateMachineThread.joinable()) {
				m_updateMachineThread.join();
			}
		}

		void DashboardMachineObserver::publishMachinesList() {
			nlohmann::json publishData = nlohmann::json::array();
			for (auto &machineOnline : m_onlineMachines) {

				nlohmann::json identificationAsJson;
				isOnline(machineOnline.first, identificationAsJson);

				if (!identificationAsJson.empty()) {
					publishData.push_back(identificationAsJson);
				}
			}

			std::stringstream stream;
			stream << "/umati/list/machineList";
			m_pPublisher->Publish(stream.str(), publishData.dump(0));
		}

		std::string DashboardMachineObserver::getTypeName(const ModelOpcUa::NodeId_t &nodeId) {
			return m_pDataClient->readNodeBrowseName(const_cast<ModelOpcUa::NodeId_t &>(nodeId));
		}

		void DashboardMachineObserver::addMachine(ModelOpcUa::BrowseResult_t machine) {
			try {
				LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:"
						  << static_cast<std::string>(machine.NodeId);

				auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher);

				MachineInformation_t machineInformation;
				machineInformation.NamespaceURI = machine.NodeId.Uri;
				machineInformation.StartNodeId = machine.NodeId;
				machineInformation.MachineName = machine.BrowseName.Name;


				std::shared_ptr<ModelOpcUa::StructureNode> p_type = getTypeOfNamespace(machine.NodeId);
				machineInformation.Specification = p_type->SpecifiedBrowseName.Name;

				std::stringstream topic;
				std::string base64ProductInstanceUri = Util::StringUtils::base64_encode(machine.NodeId.Uri, true);
				topic << "/umati" << Umati::MachineObserver::DashboardMachineObserver::getMachineSubtopic(p_type,
																										  base64ProductInstanceUri);
				pDashClient->addDataSet(
						{machineInformation.NamespaceURI, machine.NodeId.Id},
						p_type,
						topic.str()
				);

				LOG(INFO) << "Read model finished";

				{
					std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
					m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
					m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
					m_machineNames.insert(std::make_pair(machine.NodeId, machine.BrowseName.Name));
				}
			}
			catch (const Umati::Exceptions::OpcUaException &ex) {
				LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
						   << " NodeId:" << static_cast<std::string>(machine.NodeId) <<
						   "OpcUa Error: " << ex.what();

				throw Exceptions::MachineInvalidException(static_cast<std::string>(machine.NodeId));
			}
		}

		std::shared_ptr<ModelOpcUa::StructureNode>
		DashboardMachineObserver::getTypeOfNamespace(const ModelOpcUa::NodeId_t &nodeId) const {
			uint machineTypeNamespaceIndex = m_pDataClient->GetImplementedNamespaceIndex(nodeId);

			std::string typeName =
					m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceUri + ";" +
					m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceType;
			auto typePair = m_pDataClient->m_typeMap->find(typeName);
			if (typePair == m_pDataClient->m_typeMap->end()) {
				LOG(ERROR) << "Unable to find " << typeName << " in typeMap";
			}
			return typePair->second;
		}

		std::shared_ptr<ModelOpcUa::StructureNode>
		DashboardMachineObserver::getIdentificationTypeOfNamespace(const ModelOpcUa::NodeId_t &nodeId) const {
			uint machineTypeNamespaceIndex = m_pDataClient->GetImplementedNamespaceIndex(nodeId);
			std::string idType = m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceIdentificationType;
			std::string identificationTypeName =
					m_pDataClient->m_availableObjectTypeNamespaces[machineTypeNamespaceIndex].NamespaceUri + ";" +
					idType;
			auto typePair = m_pDataClient->m_typeMap->find(identificationTypeName);
			if (typePair == m_pDataClient->m_typeMap->end()) {
				LOG(ERROR) << "Unable to find " << identificationTypeName << " for namespace index "
						   << machineTypeNamespaceIndex << " in typeMap";
				throw Exceptions::MachineInvalidException("IdentificationType not found, probably because namesapce " +
														  std::to_string(machineTypeNamespaceIndex) +
														  " is not in the config");
			} else {
				// LOG(INFO) << "Found " << identificationTypeName << " for namespace index " << machineTypeNamespaceIndex << " in typeMap";
			}
			return typePair->second;
		}

		void DashboardMachineObserver::removeMachine(ModelOpcUa::BrowseResult_t machine) {
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			LOG(INFO) << "Remove Machine: " << machine.BrowseName.Name << " NodeId:"
					  << static_cast<std::string>(machine.NodeId);
			auto it = m_dashboardClients.find(machine.NodeId);
			if (it != m_dashboardClients.end()) {
				m_dashboardClients.erase(it);
			} else {
				LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machine.NodeId) << "'";
			}

			auto itOnlineMachines = m_onlineMachines.find(machine.NodeId);
			if (itOnlineMachines != m_onlineMachines.end()) {
				LOG(INFO) << "Erasing online machine";
				m_onlineMachines.erase(itOnlineMachines);
				LOG(INFO) << "Online machine erased";
			} else {
				LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machine.NodeId) << "'";
			}
		}

		bool
		DashboardMachineObserver::isOnline(const ModelOpcUa::NodeId_t &machineNodeId,
										   nlohmann::json &identificationAsJson) {
			try {
				std::shared_ptr<ModelOpcUa::StructureNode> p_type = getIdentificationTypeOfNamespace(machineNodeId);

				std::string typeName = p_type->SpecifiedBrowseName.Uri + ";" + p_type->SpecifiedBrowseName.Name;
				auto typeIt = m_pDataClient->m_nameToId->find(typeName);

				if (typeIt != m_pDataClient->m_nameToId->end()) {
					ModelOpcUa::NodeId_t type = typeIt->second;

					std::list<ModelOpcUa::BrowseResult_t> identification =
							m_pDataClient->BrowseHasComponent(machineNodeId, type);
					if (!identification.empty()) {
						LOG(INFO) << "Found component of type " << type.Uri << ";" << type.Id << " in "
								  << machineNodeId.Uri << ";" << machineNodeId.Id;

						browseIdentificationValues(machineNodeId, identification,
												   identificationAsJson);
						if (!identificationAsJson.empty()) {
							return true;
						} else {
							LOG(INFO) << "Identification JSON empty";
						}
					} else {
						LOG(INFO) << "Identification empty, couldn't find component of type " << type.Uri << ";"
								  << type.Id << " in " << machineNodeId.Uri << ";" << machineNodeId.Id;
					}
				} else {
					LOG(INFO) << "Unable to find type " << typeName << " in nameToId";
				}
			}
			catch (std::exception &ex) {
				LOG(ERROR) << ex.what();
			}
			return false;
		}

		void DashboardMachineObserver::browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId,
																  std::list<ModelOpcUa::BrowseResult_t> &identification,
																  nlohmann::json &identificationAsJson) const {
			std::vector<nlohmann::json> identificationListValues;

			std::shared_ptr<ModelOpcUa::StructureNode> p_type = getTypeOfNamespace(identification.front().NodeId);
			std::list<ModelOpcUa::NodeId_t> identificationNodes;
			std::vector<std::string> identificationValueKeys;

			m_pDataClient->FillIdentificationValuesFromBrowseResult(identification, identificationNodes,
																	identificationValueKeys);

			identificationListValues = m_pDataClient->ReadeNodeValues(identificationNodes);
			for (uint i = 0; i < identificationListValues.size(); i++) {
				auto value = identificationListValues.at(i);
				if (value.dump(0) != "null") {
					identificationAsJson[identificationValueKeys.at(i)] = value;
				}
			}

			std::stringstream path;
			auto it = m_machineNames.find(machineNodeId);
			if (it != m_machineNames.end()) {
				std::string base64ProductInstanceUri = Util::StringUtils::base64_encode(machineNodeId.Uri, true);
				identificationAsJson["Path"] = Umati::MachineObserver::DashboardMachineObserver::getMachineSubtopic(
						p_type, base64ProductInstanceUri);
			}
		}

		std::string
		DashboardMachineObserver::getMachineSubtopic(const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
													 const std::string &namespaceUri) {
			std::string specification = p_type->SpecifiedBrowseName.Name;
			std::stringstream subtopic;
			subtopic << "/" << specification << "/" << namespaceUri;
			return subtopic.str();
		}
	}
}
