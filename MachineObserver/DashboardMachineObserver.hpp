 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */
#pragma once

#include "MachineObserver.hpp"
#include <OpcUaTypeReader.hpp>
#include <DashboardClient.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <open62541/client_subscriptions.h>

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
				std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> pOpcUaTypeReaderm,
				std::vector<ModelOpcUa::NodeId_t> machinesFilter);

			~DashboardMachineObserver() override;

			void PublishAll();
			void updateAfterModelChangeEvent(UA_ModelChangeStructureDataType* modelChangeStructureDataTypes, size_t nModelChangeStructureDataTypes);

		protected:
			void startUpdateMachineThread();

			void stopMachineUpdateThread();

			void AddSubscription();

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
				ModelOpcUa::NodeId_t Parent;
			};

			int m_publishMachinesOnline = 0;

			std::atomic_bool m_running = {false};
			std::thread m_updateMachineThread;

			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;
			std::mutex m_dashboardClients_mutex;
			std::map<ModelOpcUa::NodeId_t, std::shared_ptr<Umati::Dashboard::DashboardClient>> m_dashboardClients;
			std::map<ModelOpcUa::NodeId_t, MachineInformation_t> m_onlineMachines;
			std::map<ModelOpcUa::NodeId_t, std::string> m_machineNames;

			void browseIdentificationValues(const ModelOpcUa::NodeId_t &machineNodeId, const ModelOpcUa::NodeId_t &typeDefinition, 
											ModelOpcUa::BrowseResult_t &identification,
											nlohmann::json &identificationAsJson) const;
			///\TODO Refactor: Rename, integrate into browseIdentificationValues!?
			void FillIdentificationValuesFromBrowseResult(
				const ModelOpcUa::NodeId_t &identificationInstance,
				std::list<ModelOpcUa::NodeId_t> &identificationNodes,
				std::vector<std::string> &identificationValueKeys) const;
			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId);
		};
	} // namespace MachineObserver
} // namespace Umati
