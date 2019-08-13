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
			const int PublishMachinesOnlineResetValue = 30;
			const std::string MachinesListTopic = std::string("/umati/emo/machines/list");

			DashboardMachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
				std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
				std::string machineCacheFilename
			);
			~DashboardMachineObserver();

			void PublishAll();
		protected:

			void startUpdateMachineThread();
			void stopMachineUpdateThread();

			void publishMachinesOnline();

			// Inherit from MachineObserver
			virtual void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;
			virtual void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;

			struct MachineInformation_t
			{
				std::string DisplayName;
				std::string DisplayManufacturer;
				std::string NamespaceURI;
				std::string TopicPrefix;
				std::string LocationPlant;

				operator nlohmann::json() const
				{
					nlohmann::json ret;
					ret["DisplayName"] = DisplayName;
					ret["DisplayManufacturer"] = DisplayManufacturer;
					ret["NamespaceURI"] = NamespaceURI;
					ret["TopicPrefix"] = TopicPrefix;
					ret["LocationPlant"] = LocationPlant;

					return ret;
				}
			};

			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = false;
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map < ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;

			PublishTopicFactory m_pubTopicFactory;
		};
	}
}
