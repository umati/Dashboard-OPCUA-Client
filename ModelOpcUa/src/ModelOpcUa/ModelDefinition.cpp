
#include "ModelDefinition.hpp"
#include <cassert>

namespace ModelOpcUa {

	NodeDefinition::NodeDefinition(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		QualifiedName_t specifiedBrowseName)
		: NodeClass(nodeClass),
		ModellingRule(modellingRule),
		ReferenceType(referenceType),
		SpecifiedTypeNodeId(specifiedTypeNodeId),
		SpecifiedBrowseName(specifiedBrowseName)
	{

	}

	StructureNode::StructureNode(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		QualifiedName_t specifiedBrowseName,
		std::list<std::shared_ptr<const StructureNode>> childNodes
	)
		:
		NodeDefinition(
			nodeClass,
			modellingRule,
			referenceType,
			specifiedTypeNodeId,
			specifiedBrowseName
		),
		SpecifiedChildNodes(childNodes)
	{
	}

    StructureNode::StructureNode(ModelOpcUa::BrowseResult_t browseResult,
                                 std::list<std::shared_ptr<const StructureNode>> childNodes):
            SpecifiedChildNodes(childNodes), NodeDefinition(browseResult.NodeClass, Optional, browseResult.ReferenceTypeId, browseResult.TypeDefinition, browseResult.BrowseName)
            {

            }

    StructurePlaceholderNode::StructurePlaceholderNode(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		QualifiedName_t specifiedBrowseName,
		std::list<std::shared_ptr<const StructureNode>> childNodes,
		std::list<std::shared_ptr<const StructureNode>> possibleTypes
	)
		:
		StructureNode(
			nodeClass,
			modellingRule,
			referenceType,
			specifiedTypeNodeId,
			specifiedBrowseName,
			childNodes
		),
		PossibleTypes(possibleTypes)
	{
		assert(modellingRule == ModellingRule_t::MandatoryPlaceholder ||
			modellingRule == ModellingRule_t::OptionalPlaceholder);
	}

    StructureBiNode::StructureBiNode(BrowseResult_t browseResult,
                                         std::list<std::shared_ptr<const StructureNode>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex)
                                         : structureNode(std::make_shared<StructureNode>(browseResult, childNodes)),
                                         parent(parent),
                                         namespaceIndex(namespaceIndex)  {}

    StructureBiNode::StructureBiNode(NodeClass_t nodeClass, ModellingRule_t modellingRule, NodeId_t referenceType,
                                         NodeId_t specifiedTypeNodeId, QualifiedName_t specifiedBrowseName,
                                         std::list<std::shared_ptr<const StructureNode>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex) :
            structureNode(std::make_shared<StructureNode>(nodeClass, modellingRule, referenceType, specifiedTypeNodeId, specifiedBrowseName,childNodes)),
            parent(parent),
            namespaceIndex(namespaceIndex)
            {}
}
