#pragma once

#include "MachineObserver.hpp"
#include "PublishTopicFactory.hpp"
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
			const int PublishMachinesListResetValue = 30; // in seconds
			const std::string MachinesListTopic1 = std::string("/umati/");
			const std::string MachinesListTopic2 = std::string("/config/machines/list");


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

			void publishMachinesList();

			void publishOnlineStatus(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine, bool online);

			// Inherit from MachineObserver
			void addMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;
			void removeMachine(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;
			bool isOnline(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine) override;



			struct MachineInformation_t
			{
				ModelOpcUa::NodeId_t StartNodeId;
				std::string DisplayName;
				std::string DisplayManufacturer;
				std::string NamespaceURI;
				std::string TopicPrefix;
				std::string LocationPlant;
				std::string LocationMachine;

				operator nlohmann::json() const
				{
					nlohmann::json ret;
					ret["DisplayName"] = DisplayName;
					ret["DisplayManufacturer"] = DisplayManufacturer;
					ret["NamespaceURI"] = NamespaceURI;
					ret["TopicPrefix"] = TopicPrefix;
					ret["LocationPlant"] = LocationPlant;
					ret["LocationMachine"] = LocationMachine;

					return ret;
				}
			};

			void updateMachinesMachineData(MachineInformation_t &machineInfo);

			std::string  getValueFromValuesList(std::vector<nlohmann::json, std::allocator<nlohmann::json>>& valuesList, std::string valueName, int valueIndex, ModelOpcUa::NodeId_t startNodeId);
			void split(const std::string& inputString, std::vector<std::string>& resultContainer, char delimiter = ' ');
			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = { false };
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map < ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;

			PublishTopicFactory m_pubTopicFactory;
		};
	}
}
