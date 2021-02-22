#pragma once

#include <IDashboardDataClient.hpp>
#include <map>
#include <mutex>
#include <vector>

namespace Umati {
	namespace MachineObserver {
		class MachineObserver {
		public:
			MachineObserver(
					std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient
			);

			virtual ~MachineObserver() = 0;

			const int NumSkipAfterInvalid = 10; ///< in 10s, e.g. 5 means 5*10s = 50s
			const int NumSkipAfterOffline = 3; ///< in 10s,

		protected:
			void UpdateMachines();

			bool machineListsNotEqual(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			void recreateKnownMachineToolsMap(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			bool ignoreInvalidMachinesTemporarily(
					std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachine);

			void addNewMachine(std::pair<const ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachine);

			void removeOfflineMachines(std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &toBeRemovedMachines);

			bool canBrowseMachineList(std::list<ModelOpcUa::BrowseResult_t> &machineList);

			void findNewAndOfflineMachines(std::list<ModelOpcUa::BrowseResult_t> &machineList,
										   std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &toBeRemovedMachines,
										   std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachines);

			virtual void addMachine(ModelOpcUa::BrowseResult_t machine) = 0;

			virtual void removeMachine(ModelOpcUa::BrowseResult_t machine) = 0;

			virtual bool isOnline(
				const ModelOpcUa::NodeId_t &machineNodeId,
				nlohmann::json &identificationAsJson,
				const ModelOpcUa::NodeId_t &typeDefinition
				) = 0;

			std::shared_ptr<Dashboard::IDashboardDataClient> m_pDataClient;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> m_knownMachines;
			std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> m_knownMachineToolsMap;

			/// Blacklist of invalid machines, that will not be checked periodically
			/// The value is decremented each time the machine would be checked and will only be added, when it reaches 0 again.
			std::map<ModelOpcUa::NodeId_t, int> m_invalidMachines;

			static void logMachinesChanging(const std::string &text,
											const std::map<ModelOpcUa::NodeId_t, ModelOpcUa::BrowseResult_t> &newMachines);

			std::list<ModelOpcUa::BrowseResult_t> browseForMachines();

		};
	}
}
