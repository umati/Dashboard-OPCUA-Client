#pragma once
#include "DashboardClient.hpp"
#include <IDashboardDataClient.hpp>
#include <map>
#include <mutex>
#include <vector>
#include <set>

namespace Umati {
	namespace MachineObserver {
		class MachineObserver {
		public:
			MachineObserver(
					std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
					std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pTypeReader
			);

			virtual ~MachineObserver() = 0;

			const int NumSkipAfterInvalid = 10; ///< in 10s, e.g. 5 means 5*10s = 50s
			const int NumSkipAfterOffline = 3; ///< in 10s,

		protected:
			void UpdateMachines();

			bool machineListsNotEqual(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			void recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			bool ignoreInvalidMachinesTemporarily(const ModelOpcUa::NodeId_t &newMachineId);

			void addNewMachine(const ModelOpcUa::BrowseResult_t &newMachine);

			void removeOfflineMachines(std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines);

			bool canBrowseMachineList(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			void findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t> &machineList,
											std::set<ModelOpcUa::NodeId_t> &toBeRemovedMachines,
											std::set<ModelOpcUa::NodeId_t> &newMachines,
											std::map<ModelOpcUa::NodeId_t, nlohmann::json> &machinesIdentifications);

			virtual void addMachine(ModelOpcUa::BrowseResult_t machine) = 0;

			virtual void removeMachine(ModelOpcUa::NodeId_t machineNodeId) = 0;

			virtual bool isOnline(
				const ModelOpcUa::NodeId_t &machineNodeId,
				nlohmann::json &identificationAsJson,
				const ModelOpcUa::NodeId_t &typeDefinition
				) = 0;

			std::shared_ptr<Dashboard::IDashboardDataClient> m_pDataClient;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> m_knownMachines;
			std::set<ModelOpcUa::NodeId_t> m_knownMachineToolsSet;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::NodeId_t> m_parentOfMachine;
			std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> m_pOpcUaTypeReader;
			std::mutex m_machineIdentificationsCache_mutex;
			std::map<ModelOpcUa::NodeId_t, nlohmann::json> m_machineIdentificationsCache;

			/// Blacklist of invalid machines, that will not be checked periodically
			/// The value is decremented each time the machine would be checked and will only be added, when it reaches 0 again.
			std::map<ModelOpcUa::NodeId_t, std::pair<int, std::string>> m_invalidMachines;

			static void logMachinesChanging(const std::string &text,
											const std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachines);
			static void logMachinesChanging(const std::string &text,
											const std::set<ModelOpcUa::NodeId_t> &newMachines);

			std::list<ModelOpcUa::BrowseResult_t> browseForMachines(ModelOpcUa::NodeId_t nodeid = Umati::Dashboard::NodeId_MachinesFolder, ModelOpcUa::NodeId_t parentId = Umati::Dashboard::NodeId_MachinesFolder);
			std::list<ModelOpcUa::BrowseResult_t> findComponentsFolder(ModelOpcUa::NodeId_t nodeid);

		};
	}
}
