#pragma once

#include "IDashboardDataClient.hpp"
#include <ModelOpcUa/ModelInstance.hpp>

namespace Umati {

	namespace Dashboard {
		class DashboardClient {
		public:
			DashboardClient(std::shared_ptr<IDashboardDataClient> pDashboardDataClient);

			/// \TODO rename
			void UseDataFrom(ModelOpcUa::NodeId_t startNodeId, std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition);

		protected:
			std::shared_ptr<const ModelOpcUa::SimpleNode> TransformToNodeIds(
				const std::shared_ptr<const ModelOpcUa::StructureNode> &pTypeDefinition,
				ModelOpcUa::NodeId_t startNode
			);

			std::shared_ptr<const ModelOpcUa::PlaceholderNode> BrowsePlaceholder(
				ModelOpcUa::NodeId_t startNode,
				std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder);

			std::shared_ptr<IDashboardDataClient> m_pDashboardDataClient;
		};
	}
}
