#pragma once

#include "MachineObserver.hpp"
#include "PublishTopicFactory.hpp"
#include <DashboardClient.hpp>
#include <atomic>
#include <thread>
#include <mutex>

namespace Umati {
	namespace MachineObserver {
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

			// Inherit from MachineObserver
			virtual void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;
			virtual void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;

			std::atomic_bool m_running = false;
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;

			PublishTopicFactory m_pubTopicFactory;
		};
	}
}
