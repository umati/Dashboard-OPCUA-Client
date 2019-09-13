#include "MachineObserver.hpp"
#include <TypeDefinition/UmatiTypeDefinition.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>
#include <easylogging++.h>

#include "Exceptions/MachineInvalidException.hpp"
#include "Exceptions/MachineOfflineException.hpp"
#include "Exceptions/NoPublishTopicSet.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <Exceptions/ClientNotConnected.hpp>

namespace Umati {
	namespace MachineObserver {

		MachineObserver::MachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient
		)
			: m_pDataClient(pDataClient)
		{
		}

		std::shared_ptr<ModelOpcUa::StructureNode> MachineObserver::getMachinesModel(ModelOpcUa::QualifiedName_t qualifiedName)
		{
			auto MachineToolType = std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Object,
				ModelOpcUa::ModellingRule_t::Mandatory,
				Dashboard::TypeDefinition::OrganizesTypeNodeId,
				Dashboard::TypeDefinition::NodeIds::MachineToolType,
				qualifiedName,
				std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
			}
			);

			auto MachineToolsFolderType = std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Object,
				ModelOpcUa::ModellingRule_t::Mandatory,
				ModelOpcUa::NodeId_t{},
				Dashboard::TypeDefinition::NodeIds::MachineToolsFolderType,
				qualifiedName,
				std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>{
				MachineToolType
			}
			);
			return MachineToolsFolderType;
		}

		void MachineObserver::UpdateMachines()
		{
			std::list < Umati::Dashboard::IDashboardDataClient::BrowseResult_t > browseResults;
			try {
				browseResults = m_pDataClient->Browse(
					ModelOpcUa::NodeId_t{ Dashboard::TypeDefinition::UmatiNamespaceUri, "i=1000" },
					Dashboard::TypeDefinition::OrganizesTypeNodeId,
					Dashboard::TypeDefinition::NodeIds::MachineToolType
				);
			}
			catch (const Umati::Exceptions::OpcUaException & ex)
			{
				LOG(ERROR) << "Browse new machines failed with: " << ex.what();
				return;
			}
			catch (const Umati::Exceptions::ClientNotConnected &ex)
			{
				LOG(ERROR) << "OPC UA Client not connected." << ex.what();
				return;
			}

			std::map<ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> newMachines;
			auto removedMachines = m_knownMachines;

			for (auto &browseResult : browseResults)
			{

				// Check if Machine is known as online machine
				auto it = removedMachines.find(browseResult.NodeId);

				// Machine known
				try {
					// Check if machine is still online
					if (isOnline(browseResult))
					{
						if (it != removedMachines.end())
						{
							removedMachines.erase(it);
						}
						else
						{
							newMachines.insert(std::make_pair(browseResult.NodeId, browseResult));
						}
					}
				}
				// Cach exceptions durng CheckOnline, this will cause that the machine stay in the removedMachines list
				catch (const Umati::Exceptions::OpcUaException &)
				{
					LOG(INFO) << "Machine disconnected: '" << it->second.BrowseName.Name << "' (" << it->second.NodeId.Uri << ")";
				}

			}

			for (auto &removedMachine : removedMachines)
			{
				removeMachine(removedMachine.second);
				m_knownMachines.erase(removedMachine.first);
			}

			for (auto &newMachine : newMachines)
			{
				// Ignore known invalid machines for a specific time
				auto it = m_invalidMachines.find(newMachine.second.NodeId);
				if (it != m_invalidMachines.end())
				{
					--(it->second);
					if (it->second <= 0)
					{
						m_invalidMachines.erase(it);
					}
					else
					{
						continue;
					}
				}
				try
				{
					addMachine(newMachine.second);
					m_knownMachines.insert(newMachine);
				}
				catch (const Exceptions::MachineInvalidException &)
				{
					LOG(INFO) << "Machine invalid: " << static_cast<std::string>(newMachine.second.NodeId);
					m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterInvalid));
				}
				catch (const Exceptions::MachineOfflineException &)
				{
					LOG(INFO) << "Machine offline: " << static_cast<std::string>(newMachine.second.NodeId);
					m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterOffline));
				}
				catch (const Exceptions::NoPublishTopicSet &)
				{
					LOG(INFO) << "PublishTopic not set: " << static_cast<std::string>(newMachine.second.NodeId);
					m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterInvalid));
				}
			}

		}
	}
}
