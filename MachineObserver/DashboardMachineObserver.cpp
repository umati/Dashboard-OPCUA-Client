#include "DashboardMachineObserver.hpp"
#include <easylogging++.h>
#include "Exceptions/MachineInvalidException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <utility>
#include <Topics.hpp>
#include <IdEncode.hpp>
#include "PublishMachinesList.hpp"

namespace Umati
{
	namespace MachineObserver
	{

		DashboardMachineObserver::DashboardMachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
			std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
			std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pOpcUaTypeReader) : MachineObserver(std::move(pDataClient), std::move(pOpcUaTypeReader)),
																				   m_pPublisher(std::move(pPublisher))
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
			PublishMachinesList pubList(m_pPublisher, m_pOpcUaTypeReader->m_expectedObjectTypeNames);
			for (auto &machineOnline : m_onlineMachines)
			{
				nlohmann::json identificationAsJson;
				isOnline(machineOnline.first, identificationAsJson, machineOnline.second.TypeDefinition);

				if (!identificationAsJson.empty())
				{
					/// \todo Refactor out of here
					identificationAsJson["ParentId"] = Umati::Util::IdEncode(static_cast<std::string>(machineOnline.second.Parent));
					pubList.AddMachine(machineOnline.second.Specification, identificationAsJson);
				}
			}
			pubList.Publish();
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
				if (static_cast<std::string>(machine.NodeId).find("SurfaceTechnology") != std::string::npos) {
					//FIXME Skipping surface because it will freeze...
   					LOG(ERROR) << "Skipping surfaceTechnology";
					return;
				}
				auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher, m_pOpcUaTypeReader);
				MachineInformation_t machineInformation;
				machineInformation.NamespaceURI = machine.NodeId.Uri;
				machineInformation.StartNodeId = machine.NodeId;
				machineInformation.MachineName = machine.BrowseName.Name;
				machineInformation.TypeDefinition = machine.TypeDefinition;

				{
					auto it = m_parentOfMachine.find(machine.NodeId);
					if(it != m_parentOfMachine.end())
					{
						machineInformation.Parent = it->second;
					}
				}
				std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->typeDefinitionToStructureNode(machine.TypeDefinition);
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

		void DashboardMachineObserver::removeMachine(ModelOpcUa::NodeId_t machineNodeId)
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			m_knownMachines.erase(machineNodeId);

			LOG(INFO) << "Remove Machine with NodeId:"
					  << static_cast<std::string>(machineNodeId);
			auto it = m_dashboardClients.find(machineNodeId);
			if (it != m_dashboardClients.end())
			{
				it->second.get()->Unsubscribe(machineNodeId);
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
				std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->getIdentificationTypeStructureNode(typeDefinition);
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

						browseIdentificationValues(machineNodeId, typeDefinition, identification.front(),
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
																  const ModelOpcUa::NodeId_t &typeDefinition,
																  ModelOpcUa::BrowseResult_t &identification,
																  nlohmann::json &identificationAsJson) const
		{
			std::vector<nlohmann::json> identificationListValues;
			std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->typeDefinitionToStructureNode(typeDefinition);			
			std::list<ModelOpcUa::NodeId_t> identificationNodes;
			std::vector<std::string> identificationValueKeys;

			FillIdentificationValuesFromBrowseResult(
				identification.NodeId,
				identificationNodes,
				identificationValueKeys);

			nlohmann::json identificationData;
			identificationListValues = m_pDataClient->ReadeNodeValues(identificationNodes);
			for (size_t i = 0; i < identificationListValues.size(); i++)
			{
				auto value = identificationListValues.at(i);
				if (value != nullptr)
				{
					identificationData[identificationValueKeys.at(i)] = value;
				}
			}

			identificationAsJson["Data"] = identificationData;

			auto it = m_machineNames.find(machineNodeId);
			if (it != m_machineNames.end() && p_type != nullptr)
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
				Umati::Dashboard::IDashboardDataClient::BrowseContext_t::Variable());
			for (auto &browseResult : browseResults)
			{
				identificationValueKeys.push_back(browseResult.BrowseName.Name);
				identificationNodes.emplace_back(browseResult.NodeId);
			}
		}
		
	} // namespace MachineObserver
} // namespace Umati
