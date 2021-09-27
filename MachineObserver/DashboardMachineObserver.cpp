 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2021 (c) Frank Meerkoetter, basysKom GmbH
 */

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
			{
				std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
				for (const auto &pDashClient : m_dashboardClients)
				{
					pDashClient.second->Publish();
				}
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
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul_machines(m_dashboardClients_mutex);
			std::unique_lock<decltype(m_machineIdentificationsCache_mutex)> ul(m_machineIdentificationsCache_mutex);
			PublishMachinesList pubList(m_pPublisher, m_pOpcUaTypeReader->m_expectedObjectTypeNames, Topics::List);
			for (auto &machineOnline : m_onlineMachines)
			{
				auto it = m_machineIdentificationsCache.find(machineOnline.first);
				if(it == m_machineIdentificationsCache.end() || it->second.empty()) {
					continue;
				}
				auto identificationAsJson = it->second;
				/// \todo Refactor out of here
				identificationAsJson["ParentId"] = Umati::Util::IdEncode(static_cast<std::string>(machineOnline.second.Parent));
				pubList.AddMachine(machineOnline.second.Specification, identificationAsJson);
			}
            pubList.Publish();
            auto errors = std::vector<std::string>{"errors"};
            PublishMachinesList pubInvalidList(m_pPublisher, errors, Topics::ErrorList);
            for (auto &machineInvalid: m_invalidMachines)
            {
                auto it = m_machineIdentificationsCache.find(machineInvalid.first);
                if(it == m_machineIdentificationsCache.end() || it->second.empty()) {
                    continue;
                }
                auto identificationAsJson = it->second;
                identificationAsJson["Error"] = machineInvalid.second.second;
                pubInvalidList.AddMachine("errors", identificationAsJson);
            }
            pubInvalidList.Publish();
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

				{
					auto it = m_parentOfMachine.find(machine.NodeId);
					if(it != m_parentOfMachine.end())
					{
						machineInformation.Parent = it->second;
					}
				}

				std::shared_ptr<ModelOpcUa::StructureNode> p_type = m_pOpcUaTypeReader->typeDefinitionToStructureNode(machine.TypeDefinition);
				machineInformation.Specification = p_type->SpecifiedBrowseName.Name;
                auto it = m_dashboardClients.find(machine.NodeId);
                if (it != m_dashboardClients.end())
                {
                    it->second->Unsubscribe(machine.NodeId);
                    m_dashboardClients.erase(it);
                    LOG(INFO) << "Removed Machine with duplicated reference to parent with NodeId:"
                              << static_cast<std::string>(machine.NodeId);
                }

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

				throw Exceptions::MachineInvalidException(ex.what());
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
