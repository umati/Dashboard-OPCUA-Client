 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#pragma once
#include "IDashboardDataClient.hpp"
#include "OpcUaTypeReader.hpp"
#include "IPublisher.hpp"
#include <ModelOpcUa/ModelInstance.hpp>
#include <map>
#include <set>
#include <mutex>
namespace Umati {

	namespace Dashboard {

		/**
		* DashboardClient
		* Depends on instances of:
		* - IDashboardDataClient (like Umati::OpcUa::OpcUaClient)
		* - IPublisher (like Umati::MqttPublisher::MqttPublisher)
		*
		* How DashboardClient works
		* - While the IDashboardDataClient instance acts as a source and IPublisher as the sink.
		* - DashboardClient is initialized by DashboardMachineObserver when a machine was found. 
		* - DashboardMachineObserver calls addDataSet to integrate the machine into the 
		*   system. Besides, DashboardMachineObserver forwards the Publish() function to the 
		*   DashboardOpcUaClient which contains a thread calling the fowarded Publish() method
		*   every second. DashboardClient itself then resolves topics and payloads and forwards this
		*   to the IPublisher.
		* All further functions are protected and used internally 
		*/
		class DashboardClient {
		public:
			DashboardClient(std::shared_ptr<IDashboardDataClient> pDashboardDataClient,
							std::shared_ptr<IPublisher> pPublisher,
							std::shared_ptr<OpcUaTypeReader> pTypeReader);

			void addDataSet(
					const ModelOpcUa::NodeId_t &startNodeId,
					const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
					const std::string &channel,
					const std::string &onlineChannel);

			void Publish();

			void Unsubscribe(ModelOpcUa::NodeId_t nodeId);

			bool containsNodeId(ModelOpcUa::NodeId_t nodeId);
			void updateAddDataSet(ModelOpcUa::NodeId_t nodeId);
			void updateDeleteDataSet(ModelOpcUa::NodeId_t nodeId);


		protected:

			struct LastMessage_t {
				std::string payload;
				time_t lastSent;
			};

			struct DataSetStorage_t {
				ModelOpcUa::NodeId_t startNodeId;
				std::string channel;
				std::string onlineChannel;
				std::shared_ptr<const ModelOpcUa::SimpleNode> node;
				std::mutex values_mutex;
				std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> values;
			};

			static std::string getJson(const std::shared_ptr<DataSetStorage_t> &pDataSetStorage);

			std::shared_ptr<const ModelOpcUa::SimpleNode> TransformToNodeIds(
					ModelOpcUa::NodeId_t startNode,
					const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition
			);

			std::shared_ptr<const ModelOpcUa::PlaceholderNode> BrowsePlaceholder(
					ModelOpcUa::NodeId_t startNode,
					std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStructurePlaceholder);

			void subscribeValues(
					const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
					std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
					std::mutex &valueMap_mutex
			);

			std::vector<std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>> m_subscribedValues;
			std::shared_ptr<IDashboardDataClient> m_pDashboardDataClient;
			std::shared_ptr<IPublisher> m_pPublisher;
			std::shared_ptr<OpcUaTypeReader> m_pTypeReader;

			std::set<ModelOpcUa::NodeId_t> browsedNodes;
			std::map<const ModelOpcUa::NodeId_t, std::shared_ptr<const ModelOpcUa::SimpleNode>> browsedSimpleNodes;
			std::recursive_mutex m_dataSetMutex;
			std::list<std::shared_ptr<DataSetStorage_t>> m_dataSets;
			std::map<std::string, LastMessage_t> m_latestMessages;

			bool isMandatoryOrOptionalVariable(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode);

			void handleSubscribeChildNodes(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
										   std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
										   std::mutex &valueMap_mutex);

			void handleSubscribePlaceholderChildNode(const std::shared_ptr<const ModelOpcUa::Node> &pChildNode,
													 std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
													 std::mutex &valueMap_mutex);

			void subscribeValue(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
								std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
								std::mutex &valueMap_mutex);

			void handleSubscribeChildNode(const std::shared_ptr<const ModelOpcUa::Node> &pChildNode,
										  std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap,
										  std::mutex &valueMap_mutex);

			void preparePlaceholderNodesTypeId(
					const std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> &pStructurePlaceholder,
					std::shared_ptr<ModelOpcUa::PlaceholderNode> &pPlaceholderNode,
					std::list<ModelOpcUa::BrowseResult_t> &browseResults);

			std::shared_ptr<DataSetStorage_t> prepareDataSetStorage(const ModelOpcUa::NodeId_t &startNodeId,
																	const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
																	const std::string &channel,
																	const std::string &onlineChannel);

			bool OptionalAndMandatoryTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
													   std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
													   const std::shared_ptr<ModelOpcUa::StructureNode> &pChild);

			bool OptionalAndMandatoryPlaceholderTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
																  std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
																  const std::shared_ptr<ModelOpcUa::StructureNode> &pChild);

			void TransformToNodeIdNodeNotFoundLog(const ModelOpcUa::NodeId_t &startNode,
												  const std::shared_ptr<ModelOpcUa::StructureNode> &pChild) const;

			void deleteAndUnsubscribeNode(std::shared_ptr<const ModelOpcUa::Node> node);
		};
	}
}
