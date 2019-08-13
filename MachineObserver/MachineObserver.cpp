#include "MachineObserver.hpp"
#include <TypeDefinition/UmatiTypeDefinition.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>
#include <easylogging++.h>

#include "Exceptions/MachineInvalidException.hpp"
#include "Exceptions/MachineOfflineException.hpp"

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
			auto browseResults = m_pDataClient->Browse(
				ModelOpcUa::NodeId_t{ Dashboard::TypeDefinition::UmatiNamespaceUri, "i=1000" },
				Dashboard::TypeDefinition::OrganizesTypeNodeId,
				Dashboard::TypeDefinition::NodeIds::MachineToolType
			);

			std::map<ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> newMachines;
			auto removedMachines = m_knownMachines;

			for (auto &browseResult : browseResults)
			{
				auto it = removedMachines.find(browseResult.NodeId);
				if (it != removedMachines.end())
				{
					removedMachines.erase(it);
				}
				else
				{
					newMachines.insert(std::make_pair(browseResult.NodeId, browseResult));
				}
			}

			for (auto &removedMachine : removedMachines)
			{
				removeMachine(removedMachine.second);
			}

			for (auto &newMachine : newMachines)
			{
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
			}

		}
	}
}
