#pragma once

#include "IDashboardDataClient.hpp"
#include "IPublisher.hpp"
#include <ModelOpcUa/ModelInstance.hpp>

namespace Umati {

	namespace Dashboard {
		class DashboardClient {
		public:
			DashboardClient(std::shared_ptr<IDashboardDataClient> pDashboardDataClient,
				std::shared_ptr<IPublisher> pPublisher);
			~DashboardClient();

			/// \TODO rename
			void UseDataFrom(ModelOpcUa::NodeId_t startNodeId, std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition);
			void Publish(std::string channel);

			std::string getJson();
		protected:
			std::shared_ptr<const ModelOpcUa::SimpleNode> TransformToNodeIds(
				const std::shared_ptr<const ModelOpcUa::StructureNode> &pTypeDefinition,
				ModelOpcUa::NodeId_t startNode
			);

			std::shared_ptr<const ModelOpcUa::PlaceholderNode> BrowsePlaceholder(
				ModelOpcUa::NodeId_t startNode,
				std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder);

			void subscribeValues(const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode);

			std::shared_ptr<IDashboardDataClient> m_pDashboardDataClient;
			std::shared_ptr<IPublisher> m_pPublisher;

			/// \todo rename
			std::shared_ptr<const ModelOpcUa::SimpleNode> m_node;

			std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> m_values;
		};
	}
}
