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

			void addDataSet(
				ModelOpcUa::NodeId_t startNodeId,
				std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition,
				std::string channel);

			/// \TODO rename
			void Publish();


		protected:
			struct DataSetStorage_t
			{
				ModelOpcUa::NodeId_t startNodeId;
				std::shared_ptr<ModelOpcUa::StructureNode> pTypeDefinition;
				std::string channel;
				std::shared_ptr<const ModelOpcUa::SimpleNode> node;
				std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> values;
			};

			std::string getJson(std::shared_ptr<DataSetStorage_t> pDataSetStorage);

			std::shared_ptr<const ModelOpcUa::SimpleNode> TransformToNodeIds(
				ModelOpcUa::NodeId_t startNode,
				const std::shared_ptr<const ModelOpcUa::StructureNode> &pTypeDefinition
			);

			std::shared_ptr<const ModelOpcUa::PlaceholderNode> BrowsePlaceholder(
				ModelOpcUa::NodeId_t startNode,
				std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder);

			void subscribeValues(
				const std::shared_ptr<const ModelOpcUa::SimpleNode> pNode,
				std::map<std::shared_ptr<const ModelOpcUa::Node>, nlohmann::json> &valueMap
			);

			std::shared_ptr<IDashboardDataClient> m_pDashboardDataClient;
			std::shared_ptr<IPublisher> m_pPublisher;

			std::list<std::shared_ptr<DataSetStorage_t>> m_dataSets;
		};
	}
}
