#pragma once

#include <IDashboardDataClient.hpp>
#include <map>

namespace Umati {
	namespace MachineObserver {
		class MachineObserver {
		public:
			MachineObserver(std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient);

			std::shared_ptr<ModelOpcUa::StructureNode> getMachinesModel(
				ModelOpcUa::QualifiedName_t qualifiedName = {}
			);

			virtual ~MachineObserver() = 0 {};
		protected:
			void UpdateMachines();

			virtual void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;
			virtual void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) = 0;

			std::shared_ptr<Dashboard::IDashboardDataClient> m_pDataClient;
			std::map <ModelOpcUa::NodeId_t, Umati::Dashboard::IDashboardDataClient::BrowseResult_t> m_knownMachines;
		};
	}
}
