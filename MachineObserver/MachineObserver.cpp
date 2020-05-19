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

		MachineObserver::~MachineObserver() {}

		// _2020 something like this
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

		/**
		* index 1000 is the folder machineTools where all the machines are inside
		*/
		void MachineObserver::UpdateMachines()
		{

			/**
			* Assumes that all machines are offline / to be removed
			*/
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> toBeRemovedMachines = m_knownMachines;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> newMachines;
			std::list < ModelOpcUa::BrowseResult_t > machineToolList;

			/**
			* Browses the machineToolList and fills the list if possible
			*/
			if (!canBrowseMachineToolList(machineToolList)) {
				return;
			}

			if (machineToolListsNotEqual(machineToolList)) {

				findNewAndOfflineMachines(machineToolList, toBeRemovedMachines, newMachines);

				removeOfflineMachines(toBeRemovedMachines);

				for (auto& newMachine : newMachines)
				{
					// Ignore known invalid machines for a specific time
					if (ignoreInvalidMachinesTemporarily(newMachine)) {
						continue;
					};
					addNewMachine(newMachine);
				}
			}
		}

		bool MachineObserver::machineToolListsNotEqual(std::list<ModelOpcUa::BrowseResult_t>& machineToolList)
		{
			if (m_knownMachineToolsMap.size() != machineToolList.size()) {
				recreateKnownMachineToolsMap(machineToolList);
				return true;
			}
			for (auto& machineTool : machineToolList) {
				auto it = m_knownMachineToolsMap.find(machineTool.NodeId);
				if (it == m_knownMachineToolsMap.end()) {
					// List differs
					LOG(INFO) << "Missing an entry in machineToolList: " << machineTool.BrowseName.Uri;
					recreateKnownMachineToolsMap(machineToolList);
					return true;
				}
			}
			return false;
		}

		void MachineObserver::recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t>& machineToolList)
		{
			LOG(WARNING) << "Lists differ, recreating known machine tools map" ;
			removeOfflineMachines(m_knownMachineToolsMap);
			m_knownMachineToolsMap.clear();
			for (auto machineTool : machineToolList) {
				m_knownMachineToolsMap.insert(std::make_pair(machineTool.NodeId, machineTool));
			}
		}

		bool MachineObserver::ignoreInvalidMachinesTemporarily(std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t>& newMachine)
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
					return true;
				}
			}
			return false;
		}

		bool MachineObserver::canBrowseMachineToolList(std::list<ModelOpcUa::BrowseResult_t>& machineToolList)
		{
			/**
			* i=1000 contains the list of all available machines
			*/
			try {
				machineToolList = m_pDataClient->Browse(
					ModelOpcUa::NodeId_t{ Dashboard::TypeDefinition::UmatiNamespaceUri, "i=1000" },
					Dashboard::TypeDefinition::OrganizesTypeNodeId,
					Dashboard::TypeDefinition::NodeIds::MachineToolType
				);
			}
			catch (const Umati::Exceptions::OpcUaException & ex)
			{
				LOG(ERROR) << "Browse new machines failed with: " << ex.what();
				return false;
			}
			catch (const Umati::Exceptions::ClientNotConnected & ex)
			{
				LOG(ERROR) << "OPC UA Client not connected." << ex.what();
				return false;
			}
			return true;
		}

		void MachineObserver::findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t>& machineToolList, std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t>& toBeRemovedMachines, std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t>& newMachines)
		{
            LOG(INFO) << "Checking which machines are online / offline";

            for (auto machineTool : machineToolList)
			{

				// Check if Machine is known as online machine
				auto it = toBeRemovedMachines.find(machineTool.NodeId);

				// Machine known
				try {
					// Check if machine is still online. If so, remove it from the removed machines. If it is not on there, it must be a new machine
					if (isOnline(machineTool))
					{
						if (it != toBeRemovedMachines.end())
						{
							toBeRemovedMachines.erase(it);
						}
						else
						{
							newMachines.insert(std::make_pair(machineTool.NodeId, machineTool));
						}
					}
				}
				// Catch exceptions during CheckOnline, this will cause that the machine stay in the toBeRemovedMachines list
				catch (const Umati::Exceptions::OpcUaException&)
				{
					LOG(INFO) << "Machine disconnected: '" << it->second.BrowseName.Name << "' (" << it->second.NodeId.Uri << ")";
				}
			}

            logMachinesChanging("To be removed machines: ", toBeRemovedMachines);
            logMachinesChanging("New / Staying machines: ", newMachines);
        }

        void MachineObserver::logMachinesChanging(std::string text,
                const std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &machines) {
            std::stringstream machinesStringStream;
            for(auto& machine : machines)
            {
                machinesStringStream << machine.first.Uri << "\n";
            }
            LOG(INFO) << text << "\n" << machinesStringStream.str().c_str();
        }

        void MachineObserver::removeOfflineMachines(std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t>& toBeRemovedMachines)
		{
			/**
			* First: NodeId, Second: BrowseResult_t
			*/
            logMachinesChanging("Removing machines: ", toBeRemovedMachines);

			for (auto& toBeRemovedMachine : toBeRemovedMachines)
			{
				removeMachine(toBeRemovedMachine.second);
				m_knownMachines.erase(toBeRemovedMachine.first);
			}
		}

		void MachineObserver::addNewMachine(std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t>& newMachine)
		{
			try
			{
				addMachine(newMachine.second);
				m_knownMachines.insert(newMachine);
			}
			catch (const Exceptions::MachineInvalidException&)
			{
				LOG(INFO) << "Machine invalid: " << static_cast<std::string>(newMachine.second.NodeId);
				m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterInvalid));
			}
			catch (const Exceptions::MachineOfflineException&)
			{
				LOG(INFO) << "Machine offline: " << static_cast<std::string>(newMachine.second.NodeId);
				m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterOffline));
			}
			catch (const Exceptions::NoPublishTopicSet&)
			{
				LOG(INFO) << "PublishTopic not set: " << static_cast<std::string>(newMachine.second.NodeId);
				m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterInvalid));
			}
		}
	}
}
