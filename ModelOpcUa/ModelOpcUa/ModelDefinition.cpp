
#include "ModelDefinition.hpp"

namespace ModelOpcUa {

	NodeDefinition::NodeDefinition(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		std::string specifiedBrowseName)
		: NodeClass(nodeClass),
		ModellingRule(modellingRule),
		ReferenceType(referenceType), 
		SpecifiedTypeNodeId(SpecifiedTypeNodeId),
		SpecifiedBrowseName(specifiedBrowseName)
	{

	}

	StructureNode::StructureNode(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		std::string specifiedBrowseName,
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
		ChildNodes(childNodes)
	{
	}

	StructurePlaceholderNode::StructurePlaceholderNode(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		std::string specifiedBrowseName,
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
	}
}
