#pragma once

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Interfaces {
	class ReadAddressSpace {
	public:
		struct BrowseResult_t {
			ModelOpcUa::NodeId_t NodeId;
			ModelOpcUa::NodeId_t TypeDefinitionId;
			std::string BrowseName;
			ModelOpcUa::NodeClass_t NodeClass;
		};

		virtual std::list<BrowseResult_t>
		browseNodes(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId,
					ModelOpcUa::NodeId_t nodeTypeId) = 0;

		virtual ModelOpcUa::NodeId_t
		translateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode, std::string BrowseName) = 0;
	};
}