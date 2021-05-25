#include "MachineObserver.hpp"
#include <easylogging++.h>
#include <Exceptions/OpcUaException.hpp>
#include <Exceptions/ClientNotConnected.hpp>
#include <Exceptions/MachineOfflineException.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>
#include <utility>

namespace Umati {
	namespace MachineObserver {

		MachineObserver::MachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
				std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pTypeReader
		)
				: m_pDataClient(std::move(pDataClient)), m_pOpcUaTypeReader(std::move(pTypeReader)) {
		}

		MachineObserver::~MachineObserver() {}

		/**
		* index 1000 is the folder machineTools where all the machines are inside
		*/
		void MachineObserver::UpdateMachines() {

			/**
			* Assumes that all machines are offline / to be removed
			*/
			std::set<ModelOpcUa::NodeId_t> toBeRemovedMachines;
			for(auto & knownMachine: m_knownMachines)
			{
				toBeRemovedMachines.insert(knownMachine.first);
			}
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> newMachines;
			std::list<ModelOpcUa::BrowseResult_t> machineList;

			/**
			* Browses the machineList and fills the list if possiblenodeClassFromN
			*/
			if (!canBrowseMachineList(machineList)) {
				return;
			}

			if (machineListsNotEqual(machineList)) {

				findNewAndOfflineMachines(machineList, toBeRemovedMachines, newMachines);

				removeOfflineMachines(toBeRemovedMachines);

				for (auto &newMachine : newMachines) {
					// Ignore known invalid machines for a specific time
					if (ignoreInvalidMachinesTemporarily(newMachine)) {
						continue;
					};
					addNewMachine(newMachine);
				}
			}
		}

		bool MachineObserver::machineListsNotEqual(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
			std::set<ModelOpcUa::NodeId_t> newMachines;
			// Use a set for a quick compare, as there might be duplicates in machineList!
			for (const auto &machineTool : machineList) {
				newMachines.insert(machineTool.NodeId);
			}
			if(newMachines != m_knownMachineToolsSet)
			{
				LOG(INFO) << "Different set of machines, reset known machines.";
				recreateKnownMachineToolsMap(machineList);
				return true;
			}
			return false;
		}

		void MachineObserver::recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
			LOG(WARNING) << "Lists differ, recreating known machine tools map";
			removeOfflineMachines(m_knownMachineToolsSet);
			m_knownMachineToolsSet.clear();
			for (const auto &machineTool : machineList) {
				m_knownMachineToolsSet.insert(machineTool.NodeId);
			}
		}

		bool MachineObserver::ignoreInvalidMachinesTemporarily(
				std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachine) {
			auto it = m_invalidMachines.find(newMachine.second.NodeId);
			if (it != m_invalidMachines.end()) {
				--(it->second);
				if (it->second <= 0) {
					m_invalidMachines.erase(it); // todo or does it need to be it++?
				} else {
					return true;
				}
			}
			return false;
		}

		bool MachineObserver::canBrowseMachineList(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
			try {
				LOG(INFO) << "Searching for machines";
				machineList.empty();
				machineList = browseForMachines();
			}
			catch (const Umati::Exceptions::OpcUaException &ex) {
				LOG(ERROR) << "Browse new machines failed with: " << ex.what();
				return false;
			}
			catch (const Umati::Exceptions::ClientNotConnected &ex) {
				LOG(ERROR) << "OPC UA Client not connected." << ex.what();
				return false;
			}
			return true;
		}

		std::list<ModelOpcUa::BrowseResult_t> MachineObserver::findComponentsFolder(ModelOpcUa::NodeId_t nodeid)
		{
			std::list<ModelOpcUa::BrowseResult_t> newMachines;
			try {
			auto componentFolder = m_pDataClient->TranslateBrowsePathToNodeId(nodeid, Umati::Dashboard::QualifiedName_ComponentsFolder);
				if (!componentFolder.isNull()) {
					newMachines = browseForMachines(componentFolder, nodeid);
				}
			}
			catch (const Umati::Exceptions::OpcUaException &ex) {}
			return newMachines;
		}

		std::list<ModelOpcUa::BrowseResult_t> MachineObserver::browseForMachines(ModelOpcUa::NodeId_t nodeid, ModelOpcUa::NodeId_t parentNodeId)
		{
			std::list<ModelOpcUa::BrowseResult_t> newMachines;
			auto potentialMachines = m_pDataClient->Browse(nodeid, Dashboard::IDashboardDataClient::BrowseContext_t::Hierarchical());
			for(auto const &machine: potentialMachines) {
				try {
					auto typeDefinitionNodeId = m_pOpcUaTypeReader->getIdentificationTypeNodeId(machine.TypeDefinition);
					auto ident = m_pDataClient->BrowseWithResultTypeFilter(machine.NodeId, Dashboard::IDashboardDataClient::BrowseContext_t::Hierarchical(),
																		   typeDefinitionNodeId);
					if (!ident.empty()) {
						newMachines.push_back(machine);
						newMachines.splice(newMachines.end(), findComponentsFolder(machine.NodeId));
						m_parentOfMachine.insert(std::make_pair(machine.NodeId, parentNodeId));
					}
				} catch (const Umati::Exceptions::OpcUaException &ex) {
					LOG(INFO) << "Err " << ex.what();
				} catch (const Umati::MachineObserver::Exceptions::MachineInvalidException &ex) {
					// LOG(INFO) << ex.what();
				}
			}

			return newMachines;
		}

		void MachineObserver::findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t> &machineList,
														std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines,
														std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachines) {
			LOG(INFO) << "Checking which machines are online / offline";

			for (auto &machineTool : machineList) {

				// Check if Machine is known as online machine
				auto it = toBeRemovedMachines.find(machineTool.NodeId);

				// Machine known
				try {
					// Check if machine is still online. If so, remove it from the removed machines. If it is not on there, it must be a new machine
					nlohmann::json identificationAsJson;
					if (isOnline(machineTool.NodeId, identificationAsJson, machineTool.TypeDefinition)) {
						if (it != toBeRemovedMachines.end()) {
							toBeRemovedMachines.erase(it);// todo or does it need to be it++?
						} else {
							newMachines.insert(std::make_pair(machineTool.NodeId, machineTool));
						}
					} else {
						LOG(INFO) << "Machine " << machineTool.BrowseName.Name << " not identified as online";
					}
				}
					// Catch exceptions during CheckOnline, this will cause that the machine stay in the toBeRemovedMachines list
				catch (const Umati::Exceptions::OpcUaException &) {
					LOG(INFO) << "Machine disconnected: '" << machineTool.BrowseName.Name << "' ("
							  << machineTool.NodeId.Uri << ")";
				}
			}

			logMachinesChanging("To be removed machines: ", toBeRemovedMachines);
			logMachinesChanging("New / Staying machines: ", newMachines);
		}

		void MachineObserver::logMachinesChanging(const std::string &text,
												  const std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &machines) {
			std::stringstream machinesStringStream;
			for (auto &machine : machines) {
				machinesStringStream << machine.first.Uri << "\n";
			}
			LOG(INFO) << text << "\n" << machinesStringStream.str().c_str();
		}

		void MachineObserver::logMachinesChanging(const std::string &text,
												  const std::set<ModelOpcUa::NodeId_t> &machines) {
			std::stringstream machinesStringStream;
			for (auto &machine : machines) {
				machinesStringStream << machine.Uri << "\n";
			}
			LOG(INFO) << text << "\n" << machinesStringStream.str().c_str();
		}

		void MachineObserver::removeOfflineMachines(
				std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines) {

			logMachinesChanging("Removing machines: ", toBeRemovedMachines);

			for (auto &toBeRemovedMachine : toBeRemovedMachines) {
				removeMachine(toBeRemovedMachine);
			}
		}

		void
		MachineObserver::addNewMachine(std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachine) {
			try {
				addMachine(newMachine.second);
				m_knownMachines.insert(newMachine);
			}
			catch (const Exceptions::MachineInvalidException &) {
				LOG(INFO) << "Machine invalid: " << static_cast<std::string>(newMachine.second.NodeId);
				m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterInvalid));
			}
			catch (const Exceptions::MachineOfflineException &) {
				LOG(INFO) << "Machine offline: " << static_cast<std::string>(newMachine.second.NodeId);
				m_invalidMachines.insert(std::make_pair(newMachine.second.NodeId, NumSkipAfterOffline));
			}
		}
	}
}
