#pragma once
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati {

	namespace Dashboard
	{
		class IDashboardDataClient
		{
		public:

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

			//Subscribe();

		};
	}
}
