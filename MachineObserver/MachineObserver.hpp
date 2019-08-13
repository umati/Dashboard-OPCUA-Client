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

			virtual ~MachineObserver() = 0 {};

			const int NumSkipAfterInvalid = 100;
			const int NumSkipAfterOffline = 100;

		protected:
			void UpdateMachines();

			virtual void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;
			virtual void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;

			std::shared_ptr<Dashboard::IDashboardDataClient> m_pDataClient;
			std::map <ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> m_knownMachines;

			/// Blacklist of invalid machines, that will not be checked periodically
			/// The value is decremented each time the machine would be checked and will only be added, when it reaches 0 again.
			std::map< ModelOpcUa::NodeId_t, int> m_invalidMachines;
		};
	}
}
