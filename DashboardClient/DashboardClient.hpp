#pragma once

#include "IDashboardDataClient.hpp"
#include "IPublisher.hpp"
#include <ModelOpcUa/ModelInstance.hpp>
#include <map>

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
				std::shared_ptr<IPublisher> pPublisher);
			~DashboardClient();

			void addDataSet(
				const ModelOpcUa::NodeId_t& startNodeId,
				const std::shared_ptr<ModelOpcUa::StructureNode>& pTypeDefinition,
				const std::string& channel);

			void Publish();


		protected:

            struct LastMessage_t {
                std::string payload;
                time_t lastSent;
            };

			struct DataSetStorage_t
			{
				ModelOpcUa::NodeId_t startNodeId;
                std::string channel;
				std::shared_ptr<const ModelOpcUa::SimpleNode> node;
				std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> values;
			};

			static std::string getJson(const std::shared_ptr<DataSetStorage_t>& pDataSetStorage, std::string topicName);

			std::shared_ptr<const ModelOpcUa::SimpleNode> TransformToNodeIds(
				ModelOpcUa::NodeId_t startNode,
				const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition
			);

			std::shared_ptr<const ModelOpcUa::PlaceholderNode> BrowsePlaceholder(
				ModelOpcUa::NodeId_t startNode,
				std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStructurePlaceholder);

			void subscribeValues(
				const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
				std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap
			);

			std::vector<std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>> m_subscribedValues;
			std::shared_ptr<IDashboardDataClient> m_pDashboardDataClient;
			std::shared_ptr<IPublisher> m_pPublisher;

			std::list<std::shared_ptr<DataSetStorage_t>> m_dataSets;
			std::map<std::string, LastMessage_t> m_latestMessages;

            bool isMandatoryOrOptionalVariable(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode);

            void handleSubscribeChildNodes(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
                                           std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap);

            bool handleSubscribePlaceholderChildNode(std::shared_ptr<const ModelOpcUa::Node> pChildNode,
                                                     std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap);

            void subscribeValue(const std::shared_ptr<const ModelOpcUa::SimpleNode> &pNode,
                                std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap);

            bool handleSubscribeChildNode(std::shared_ptr<const ModelOpcUa::Node> pChildNode,
                                          std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap);

            void preparePlaceholderNodesTypeId(
                    const std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> &pStructurePlaceholder,
                    std::shared_ptr<ModelOpcUa::PlaceholderNode> &pPlaceholderNode,
                    const std::list<ModelOpcUa::BrowseResult_t> &browseResults);

            std::shared_ptr<DataSetStorage_t> prepareDataSetStorage(const ModelOpcUa::NodeId_t &startNodeId,
                                                                    const std::shared_ptr<ModelOpcUa::StructureNode> &pTypeDefinition,
                                                                    const std::string &channel);

            bool OptionalAndMandatoryTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
                                                       std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
                                                       const std::shared_ptr<ModelOpcUa::StructureNode> &pChild);

            bool OptionalAndMandatoryPlaceholderTransformToNodeId(const ModelOpcUa::NodeId_t &startNode,
                                                                  std::list<std::shared_ptr<const ModelOpcUa::Node>> &foundChildNodes,
                                                                  const std::shared_ptr<ModelOpcUa::StructureNode> &pChild);

            void TransformToNodeIdNodeNotFoundLog(const ModelOpcUa::NodeId_t &startNode,
                                                  const std::shared_ptr<ModelOpcUa::StructureNode> &pChild) const;
        };
	}
}
