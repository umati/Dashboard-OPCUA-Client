#pragma once

#include <IDashboardDataClient.hpp>
#include <map>
#include "IMachineCache.hpp"

namespace Umati {
	namespace MachineObserver {
		class MachineObserver {
		public:
			MachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient
				);

			std::shared_ptr<ModelOpcUa::StructureNode> getMachinesModel(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			virtual ~MachineObserver() = 0;

			const int NumSkipAfterInvalid = 10; ///< in 10s, e.g. 5 means 5*10s = 50s
			const int NumSkipAfterOffline = 3; ///< in 10s,

		protected:
			void UpdateMachines();

			bool machineToolListsNotEqual(std::list<Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& machineToolList);

			void recreateKnownMachineToolsMap(std::list<Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& machineToolList);

			bool ignoreInvalidMachinesTemporarily(std::pair<const ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& newMachine);

			void addNewMachine(std::pair<const ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& newMachine);

			void removeOfflineMachines(std::map<ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& toBeRemovedMachines);

			bool canBrowseMachineToolList(std::list<Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& machineToolList);

			void findNewAndOfflineMachines(std::list<Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& machineToolList, std::map<ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& toBeRemovedMachines, std::map<ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t>& newMachines);

			virtual void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;
			virtual void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;
			virtual bool isOnline(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;

			std::shared_ptr<Dashboard::IDashboardDataClient> m_pDataClient;
			std::map <ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> m_knownMachines;
			std::map <ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> m_knownMachineToolsMap;

			/// Blacklist of invalid machines, that will not be checked periodically
			/// The value is decremented each time the machine would be checked and will only be added, when it reaches 0 again.
			std::map< ModelOpcUa::NodeId_t, int> m_invalidMachines;
		};
	}
}
