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

			bool isOnline(const ModelOpcUa::NodeId_t &machineNodeId, nlohmann::json &identificationAsJson) override;

			struct MachineInformation_t {
				ModelOpcUa::NodeId_t StartNodeId;
				std::string NamespaceURI;
				std::string Specification;
				std::string MachineName;
			};

			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = {false};
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;

			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map<ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;
			std::map<ModelOpcUa::NodeId_t, std::string> m_machineNames;

			void browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId,
											std::list<ModelOpcUa::BrowseResult_t> &identification,
											UaReferenceDescriptions &referenceDescriptions,
											nlohmann::json &identificationAsJson) const;

			std::shared_ptr<ModelOpcUa::StructureNode> getTypeOfNamespace(ModelOpcUa::NodeId_t nodeId) const;

			std::shared_ptr<ModelOpcUa::StructureNode>
			getIdentificationTypeOfNamespace(ModelOpcUa::NodeId_t nodeId) const;

			uint getImplementedNamespaceIndex(const ModelOpcUa::NodeId_t &nodeId) const;

			static std::string getMachineSubtopic(const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
												  const std::string &namespaceUri);

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId);
		};
	}
}
