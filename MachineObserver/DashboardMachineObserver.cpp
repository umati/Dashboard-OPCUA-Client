#include "DashboardMachineObserver.hpp"
#include <easylogging++.h>
#include "Exceptions/MachineInvalidException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <utility>
#include <Topics.hpp>
#include <IdEncode.hpp>

namespace Umati
{
	namespace MachineObserver
	{

		DashboardMachineObserver::DashboardMachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
			std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
			std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pOpcUaTypeReader) : MachineObserver(std::move(pDataClient)),
																				   m_pPublisher(std::move(pPublisher)),
																				   m_pOpcUaTypeReader(pOpcUaTypeReader)
		{
			startUpdateMachineThread();
		}

		DashboardMachineObserver::~DashboardMachineObserver()
		{
			stopMachineUpdateThread();
		}

		void DashboardMachineObserver::PublishAll()
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			for (const auto &pDashClient : m_dashboardClients)
			{
				pDashClient.second->Publish();
			}

			// Publish online machines every 30th publish
			if (++m_publishMachinesOnline >= 30)
			{
				this->publishMachinesList();
				m_publishMachinesOnline = 0;
			}

		}

		void DashboardMachineObserver::startUpdateMachineThread()
		{
			if (m_running)
			{
				LOG(INFO) << "Machine update thread already running";
				return;
			}

			auto func = [this]() {
				int cnt = 0;
				while (this->m_running)
				{
					if ((cnt % 10) == 0)
					{
						this->UpdateMachines();
					}

					++cnt;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			};
			m_running = true;
			m_updateMachineThread = std::thread(func);
		}

		void DashboardMachineObserver::stopMachineUpdateThread()
		{
			m_running = false;
			if (m_updateMachineThread.joinable())
			{
				m_updateMachineThread.join();
			}
		}

		void DashboardMachineObserver::publishMachinesList()
		{
			nlohmann::json publishData = nlohmann::json::array();
			for (auto &machineOnline : m_onlineMachines)
			{

				nlohmann::json identificationAsJson;
				isOnline(machineOnline.first, identificationAsJson, machineOnline.second.TypeDefinition);

				if (!identificationAsJson.empty())
				{
					publishData.push_back(identificationAsJson);
				}
			}

			m_pPublisher->Publish(Topics::List("MachineTools"), publishData.dump(0));
		}

		std::string DashboardMachineObserver::getTypeName(const ModelOpcUa::NodeId_t &nodeId)
		{
			return m_pDataClient->readNodeBrowseName(const_cast<ModelOpcUa::NodeId_t &>(nodeId));
		}

		void DashboardMachineObserver::addMachine(ModelOpcUa::BrowseResult_t machine)
		{
			try
			{
				LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:"
						  << static_cast<std::string>(machine.NodeId);

				auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher, m_pOpcUaTypeReader);

				MachineInformation_t machineInformation;
				machineInformation.NamespaceURI = machine.NodeId.Uri;
				machineInformation.StartNodeId = machine.NodeId;
				machineInformation.MachineName = machine.BrowseName.Name;
				machineInformation.TypeDefinition = machine.TypeDefinition;

				std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->getTypeOfNamespace(machine.TypeDefinition.Uri);
				machineInformation.Specification = p_type->SpecifiedBrowseName.Name;

				pDashClient->addDataSet(
					{machineInformation.NamespaceURI, machine.NodeId.Id},
					p_type,
					Topics::Machine(p_type, static_cast<std::string>(machine.NodeId)));

				LOG(INFO) << "Read model finished";

				{
					std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
					m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
					m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
					m_machineNames.insert(std::make_pair(machine.NodeId, machine.BrowseName.Name));
				}
			}
			catch (const Umati::Exceptions::OpcUaException &ex)
			{
				LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
						   << " NodeId:" << static_cast<std::string>(machine.NodeId) << "OpcUa Error: " << ex.what();

				throw Exceptions::MachineInvalidException(static_cast<std::string>(machine.NodeId));
			}
		}

		std::shared_ptr<ModelOpcUa::StructureNode>
		DashboardMachineObserver::getIdentificationTypeOfNamespace(const ModelOpcUa::NodeId_t &typeDefinition) const
		{
			std::string identificationTypeName;
			try
			{
				auto el = m_pOpcUaTypeReader->m_availableObjectTypeNamespaces.at(typeDefinition.Uri);
				identificationTypeName = el.NamespaceUri + ";" + el.NamespaceIdentificationType;
			}
			catch (std::out_of_range &ex)
			{
				LOG(ERROR) << "Could not found type namespace for URI: " << typeDefinition.Uri << std::endl;
				throw Exceptions::MachineInvalidException("Could not found type namespace for URI");
			}
			auto typePair = m_pOpcUaTypeReader->m_typeMap->find(identificationTypeName);
			if (typePair == m_pOpcUaTypeReader->m_typeMap->end())
			{
				LOG(ERROR) << "Unable to find " << identificationTypeName << " for namespace "
						   << typeDefinition.Uri << " in typeMap";
				throw Exceptions::MachineInvalidException("IdentificationType not found, probably because namesapce " +
														  typeDefinition.Uri +
														  " is not in the config");
			}
			return typePair->second;
		}

		void DashboardMachineObserver::removeMachine(ModelOpcUa::NodeId_t machineNodeId)
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			m_knownMachines.erase(machineNodeId);

			LOG(INFO) << "Remove Machine with NodeId:"
					  << static_cast<std::string>(machineNodeId);
			auto it = m_dashboardClients.find(machineNodeId);
			if (it != m_dashboardClients.end())
			{
				m_dashboardClients.erase(it);
			}
			else
			{
				LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machineNodeId) << "'";
			}

			auto itOnlineMachines = m_onlineMachines.find(machineNodeId);
			if (itOnlineMachines != m_onlineMachines.end())
			{
				m_onlineMachines.erase(itOnlineMachines);
				LOG(INFO) << "Online machine erased";
			}
			else
			{
				LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machineNodeId) << "'";
			}
		}

		bool
		DashboardMachineObserver::isOnline(
			const ModelOpcUa::NodeId_t &machineNodeId,
			nlohmann::json &identificationAsJson,
			const ModelOpcUa::NodeId_t &typeDefinition)
		{
			try
			{
				std::shared_ptr<ModelOpcUa::StructureNode> p_type = getIdentificationTypeOfNamespace(typeDefinition);

				std::string typeName = p_type->SpecifiedBrowseName.Uri + ";" + p_type->SpecifiedBrowseName.Name;
				auto typeIt = m_pOpcUaTypeReader->m_nameToId->find(typeName);
				/// \todo Should be p_Type.specifiedTypeId ?
				if (typeIt != m_pOpcUaTypeReader->m_nameToId->end())
				{
					ModelOpcUa::NodeId_t type = typeIt->second;

					std::list<ModelOpcUa::BrowseResult_t> identification =
						m_pDataClient->BrowseHasComponent(machineNodeId, type);
					if (!identification.empty())
					{
						LOG(DEBUG) << "Found component of type " << type.Uri << ";" << type.Id << " in "
								   << machineNodeId.Uri << ";" << machineNodeId.Id;

						browseIdentificationValues(machineNodeId, identification.front(),
												   identificationAsJson);
						if (!identificationAsJson.empty())
						{
							return true;
						}
						else
						{
							LOG(DEBUG) << "Identification JSON empty";
						}
					}
					else
					{
						LOG(INFO) << "Identification empty, couldn't find component of type " << type.Uri << ";"
								  << type.Id << " in " << machineNodeId.Uri << ";" << machineNodeId.Id;
					}
				}
				else
				{
					LOG(INFO) << "Unable to find type " << typeName << " in nameToId";
				}
			}
			catch (std::exception &ex)
			{
				LOG(ERROR) << ex.what();
			}
			return false;
		}

		void DashboardMachineObserver::browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId,
																  ModelOpcUa::BrowseResult_t &identification,
																  nlohmann::json &identificationAsJson) const
		{
			std::vector<nlohmann::json> identificationListValues;

			std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->getTypeOfNamespace(identification.TypeDefinition.Uri);
			std::list<ModelOpcUa::NodeId_t> identificationNodes;
			std::vector<std::string> identificationValueKeys;

			FillIdentificationValuesFromBrowseResult(
				identification.NodeId,
				identificationNodes,
				identificationValueKeys);

			nlohmann::json identificationData;
			identificationListValues = m_pDataClient->ReadeNodeValues(identificationNodes);
			for (uint i = 0; i < identificationListValues.size(); i++)
			{
				auto value = identificationListValues.at(i);
				if (value != nullptr)
				{
					identificationData[identificationValueKeys.at(i)] = value;
				}
			}

			identificationAsJson["Data"] = identificationData;

			auto it = m_machineNames.find(machineNodeId);
			if (it != m_machineNames.end())
			{
				identificationAsJson["Topic"] = Topics::Machine(p_type, static_cast<std::string>(machineNodeId));
				identificationAsJson["MachineId"] = Umati::Util::IdEncode(static_cast<std::string>(machineNodeId));
				identificationAsJson["TypeDefinition"] = p_type->SpecifiedBrowseName.Name;
			}
		}

		void DashboardMachineObserver::FillIdentificationValuesFromBrowseResult(
			const ModelOpcUa::NodeId_t &identificationInstance,
			std::list<ModelOpcUa::NodeId_t> &identificationNodes,
			std::vector<std::string> &identificationValueKeys) const
		{
			///\TODO browse by type definition
			auto browseResults = m_pDataClient->Browse(
				identificationInstance,
				Umati::Dashboard::IDashboardDataClient::BrowseContext_t::ObjectAndVariable());
			for (auto &browseResult : browseResults)
			{
				identificationValueKeys.push_back(browseResult.BrowseName.Name);
				identificationNodes.emplace_back(browseResult.NodeId);
			}
		}

	} // namespace MachineObserver
} // namespace Umati
