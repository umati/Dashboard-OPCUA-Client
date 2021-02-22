#include "MachineObserver.hpp"
#include <easylogging++.h>

#include "Exceptions/MachineInvalidException.hpp"
#include "Exceptions/MachineOfflineException.hpp"
#include <Exceptions/OpcUaException.hpp>
#include <Exceptions/ClientNotConnected.hpp>
#include <TypeDefinition/UmatiTypeNodeIds.hpp>
#include <utility>

namespace Umati {
	namespace MachineObserver {

		MachineObserver::MachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient
		)
				: m_pDataClient(std::move(pDataClient)) {
		}

		MachineObserver::~MachineObserver() {}

		/**
		* index 1000 is the folder machineTools where all the machines are inside
		*/
		void MachineObserver::UpdateMachines() {

			/**
			* Assumes that all machines are offline / to be removed
			*/
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> toBeRemovedMachines = m_knownMachines;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> newMachines;
			std::list<ModelOpcUa::BrowseResult_t> machineList;

			/**
			* Browses the machineList and fills the list if possible
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
			if (m_knownMachineToolsMap.size() != machineList.size()) {
				LOG(INFO) << "Size differs, known != new: " << m_knownMachineToolsMap.size() << " != " << machineList.size();
				recreateKnownMachineToolsMap(machineList);
				return true;
			}
			for (auto &machineTool : machineList) {
				auto it = m_knownMachineToolsMap.find(machineTool.NodeId);
				if (it == m_knownMachineToolsMap.end()) {
					// List differs
					LOG(INFO) << "Missing an entry in machineList: " << machineTool.BrowseName.Uri;
					recreateKnownMachineToolsMap(machineList);
					return true;
				}
			}
			return false;
		}

		void MachineObserver::recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t> &machineList) {
			LOG(WARNING) << "Lists differ, recreating known machine tools map";
			removeOfflineMachines(m_knownMachineToolsMap);
			m_knownMachineToolsMap.clear();
			for (const auto &machineTool : machineList) {
				m_knownMachineToolsMap.insert(std::make_pair(machineTool.NodeId, machineTool));
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

		std::list<ModelOpcUa::BrowseResult_t> MachineObserver::browseForMachines()
		{
			Umati::Dashboard::IDashboardDataClient::BrowseContext_t ctx;
			ctx.referenceTypeId = Umati::Dashboard::NodeId_HierarchicalReferences;
			ctx.nodeClassMask = (std::uint32_t) Umati::Dashboard::IDashboardDataClient::BrowseContext_t::NodeClassMask::OBJECT;
			return m_pDataClient->Browse(
					Umati::Dashboard::NodeId_MachinesFolder,
					ctx
				);
		}

		void MachineObserver::findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t> &machineList,
														std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &toBeRemovedMachines,
														std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachines) {
			LOG(INFO) << "Checking which machines are online / offline";

			for (const auto &machineTool : machineList) {

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
					LOG(INFO) << "Machine disconnected: '" << it->second.BrowseName.Name << "' ("
							  << it->second.NodeId.Uri << ")";
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

		void MachineObserver::removeOfflineMachines(
				std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &toBeRemovedMachines) {
			/**
			* First: NodeId, Second: BrowseResult_t
			*/
			logMachinesChanging("Removing machines: ", toBeRemovedMachines);

			for (auto &toBeRemovedMachine : toBeRemovedMachines) {
				removeMachine(toBeRemovedMachine.second);
				m_knownMachines.erase(toBeRemovedMachine.first);
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
