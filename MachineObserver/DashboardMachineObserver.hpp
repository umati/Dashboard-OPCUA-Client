#pragma once

#include "MachineObserver.hpp"
#include <OpcUaTypeReader.hpp>
#include <DashboardClient.hpp>
#include <atomic>
#include <thread>
#include <mutex>

namespace Umati
{
	namespace MachineObserver
	{
		/**
		* Depends on an iPublisher (e.g. mqtt client) to publish a machine list and online states of machines and
		* an iDashboardDataClient (e.g. Umati::OpcUa::OpcUaClient) to read the machineList. It inherits stuff from MachineObserver
		* and is itself used by the main function DashboardOpcUaClient.
		*/
		class DashboardMachineObserver : public MachineObserver
		{
		public:
			DashboardMachineObserver(
				std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
				std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
				std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pOpcUaTypeReader);

			~DashboardMachineObserver() override;

			void PublishAll();

		protected:
			void startUpdateMachineThread();

			void stopMachineUpdateThread();

			void publishMachinesList();

			// Inherit from MachineObserver
			void addMachine(ModelOpcUa::BrowseResult_t machine) override;

			void removeMachine(ModelOpcUa::NodeId_t machineNodeId) override;

			bool isOnline(
				const ModelOpcUa::NodeId_t &machineNodeId,
				nlohmann::json &identificationAsJson,
				const ModelOpcUa::NodeId_t &typeDefinition) override;

			struct MachineInformation_t
			{
				ModelOpcUa::NodeId_t StartNodeId;
				std::string NamespaceURI;
				std::string Specification;
				std::string MachineName;
				ModelOpcUa::NodeId_t TypeDefinition;
			};

			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = {false};
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;
			std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> m_pOpcUaTypeReader;
			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map<ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;
			std::map<ModelOpcUa::NodeId_t, std::string> m_machineNames;

			void browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId,
											ModelOpcUa::BrowseResult_t &identification,
											nlohmann::json &identificationAsJson) const;
			///\TODO Refactor: Rename, integrate into browseIdentificationValues!?
			void FillIdentificationValuesFromBrowseResult(
				const ModelOpcUa::NodeId_t &identificationInstance,
				std::list<ModelOpcUa::NodeId_t> &identificationNodes,
				std::vector<std::string> &identificationValueKeys) const;

			std::shared_ptr<ModelOpcUa::StructureNode> getTypeOfNamespace(const ModelOpcUa::NodeId_t &nodeId) const;

			std::shared_ptr<ModelOpcUa::StructureNode>
			getIdentificationTypeOfNamespace(const ModelOpcUa::NodeId_t &typeDefinition) const;

			static std::string getMachineSubtopic(const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
												  const std::string &namespaceUri);

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId);
		};
	} // namespace MachineObserver
} // namespace Umati
