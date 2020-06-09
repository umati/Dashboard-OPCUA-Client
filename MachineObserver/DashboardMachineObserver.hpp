#pragma once

#include "MachineObserver.hpp"
#include <DashboardClient.hpp>
#include <atomic>
#include <thread>
#include <mutex>

namespace Umati {
	namespace MachineObserver {
		/**
		* Depends on an iPublisher (e.g. mqtt client) to publish a machine list and online states of machines and
		* an iDashboardDataClient (e.g. Umati::OpcUa::OpcUaClient) to read the machineList. It inherits stuff from MachineObserver
		* and is itself used by the main function DashboardOpcUaClient.
		*/
		class DashboardMachineObserver : public MachineObserver {
		public:
			DashboardMachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
				std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher
			);
			~DashboardMachineObserver();

			void PublishAll();
		protected:

			void startUpdateMachineThread();
			void stopMachineUpdateThread();

			void publishMachinesList();


			// Inherit from MachineObserver
			void addMachine(ModelOpcUa::BrowseResult_t machine) override;
			void removeMachine(ModelOpcUa::BrowseResult_t machine) override;
			bool isOnline(const ModelOpcUa::BrowseResult_t& machine, const ModelOpcUa::NodeId_t& type) override;

			struct MachineInformation_t
			{
				ModelOpcUa::NodeId_t StartNodeId;
				std::string NamespaceURI;
				std::string Fair;
                nlohmann::json IdentificationJson;
			};

			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = { false };
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map <ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;


            void browseIdentificationValues(std::list<ModelOpcUa::BrowseResult_t> &identification, int namespaceIndex,
                                            UaReferenceDescriptions &referenceDescriptions,
                                            std::vector<nlohmann::json> &identificationListValues) const;
		};
	}
}
