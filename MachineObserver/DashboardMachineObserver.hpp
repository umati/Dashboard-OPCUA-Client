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
			const int PublishMachinesListResetValue = 30; // in seconds

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


			// Inherit from MachineObserver
			void addMachine(ModelOpcUa::BrowseResult_t machine) override;
			void removeMachine(ModelOpcUa::BrowseResult_t machine) override;
			bool isOnline(ModelOpcUa::BrowseResult_t machine) override;



			struct MachineInformation_t
			{
				ModelOpcUa::NodeId_t StartNodeId;
				std::string DisplayName;
				std::string DisplayManufacturer;
				std::string NamespaceURI;
				std::string LocationPlant;
				std::string LocationMachine;

				operator nlohmann::json() const
				{
					nlohmann::json ret;
					ret["DisplayName"] = DisplayName;
					ret["DisplayManufacturer"] = DisplayManufacturer;
					ret["NamespaceURI"] = NamespaceURI;
					ret["LocationPlant"] = LocationPlant;
					ret["LocationMachine"] = LocationMachine;

					return ret;
				}
			};

			std::string  getValueFromValuesList(std::vector<nlohmann::json, std::allocator<nlohmann::json>>& valuesList, std::string valueName, int valueIndex, ModelOpcUa::NodeId_t startNodeId);
			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = { false };
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map < ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;


            void browseIdentificationValues(std::list<ModelOpcUa::BrowseResult_t> &identification, int namespaceIndex,
                                            UaReferenceDescriptions &referenceDescriptions,
                                            std::vector<nlohmann::json> &identificationListValues) const;

            std::string identificationValuesToJsonString(const UaReferenceDescriptions &referenceDescriptions,
                                                         const std::vector<nlohmann::json> &identificationListValues,
                                                         std::string &fair, std::string &manufacturer,
                                                         std::string &machine_name) const;
		};
	}
}
