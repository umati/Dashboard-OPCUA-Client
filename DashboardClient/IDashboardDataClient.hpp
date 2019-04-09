#pragma once
#include <nlohmann/json.hpp>

#include <ModelOpcUa/ModelDefinition.hpp>
#include <functional>

namespace Umati {

	namespace Dashboard
	{
		class IDashboardDataClient
		{
		public:

			typedef std::function<void(nlohmann::json value)> newValueCallbackFunction_t;

			virtual ~IDashboardDataClient() = default;

			struct BrowseResult_t
			{
				ModelOpcUa::NodeClass_t NodeClass;
				ModelOpcUa::NodeId_t NodeId;
				ModelOpcUa::NodeId_t TypeDefinition;
				ModelOpcUa::NodeId_t ReferenceTypeId;
				ModelOpcUa::QualifiedName_t BrowseName;
				//std::string DisplayName;
			};

			virtual std::list<BrowseResult_t> Browse(
				ModelOpcUa::NodeId_t startNode,
				ModelOpcUa::NodeId_t referenceTypeId,
				ModelOpcUa::NodeId_t typeDefinition
			) = 0;

			virtual ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(
				ModelOpcUa::NodeId_t startNode,
				ModelOpcUa::QualifiedName_t browseName
			) = 0;

			virtual void Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) = 0;

			virtual void UnsubscribeAll() = 0;

		};
	}
}
