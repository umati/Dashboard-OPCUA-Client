
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

    StructureNode::StructureNode(ModelOpcUa::BrowseResult_t browseResult,
                                 std::list<std::shared_ptr<const StructureNode>> childNodes, ModellingRule_t modellingRule):
            SpecifiedChildNodes(childNodes), NodeDefinition(browseResult.NodeClass, modellingRule, browseResult.ReferenceTypeId, browseResult.TypeDefinition, browseResult.BrowseName)
            {

            }

    std::string StructureNode::printType(std::shared_ptr<const StructureNode> node, std::string parentTree) {
        std::stringstream ss;
        std::stringstream ss_self;

        std::string modellingRule = "";
        std::string nodeClass = "";
        if (node->ModellingRule == ModellingRule_t::Optional) {
            modellingRule = "Optional";
        } else if (node->ModellingRule == ModellingRule_t::Mandatory) {
            modellingRule = "Mandatory";
        } else if (node->ModellingRule == ModellingRule_t::MandatoryPlaceholder) {
            modellingRule = "MandatoryPlaceholder";
        } else if (node->ModellingRule == ModellingRule_t::OptionalPlaceholder) {
            modellingRule = "OptionalPlaceholder";
        }

        if(node->NodeClass == NodeClass_t::Object) {
            nodeClass = "Object";
        } else if(node->NodeClass == NodeClass_t::Variable) {
            nodeClass = "Variable";
        } else if(node->NodeClass == NodeClass_t::Method) {
            nodeClass = "Method";
        } else if(node->NodeClass == NodeClass_t::ObjectType) {
            nodeClass = "ObjectType";
        } else if(node->NodeClass == NodeClass_t::VariableType) {
            nodeClass = "VariableType";
        } else if(node->NodeClass == NodeClass_t::ReferenceType) {
            nodeClass = "ReferenceType";
        } else if(node->NodeClass == NodeClass_t::DataType) {
            nodeClass = "DataType";
        } else if(node->NodeClass == NodeClass_t::View) {
            nodeClass = "View";
        }

        ss_self << parentTree << "->" << node->SpecifiedBrowseName.Name << " (" << nodeClass << (modellingRule == "" ? "" : "|" ) << modellingRule << ")";
        ss << ss_self.str() << std::endl;


        for (auto childNodesIterator = node->SpecifiedChildNodes.begin(); childNodesIterator != node->SpecifiedChildNodes.end(); childNodesIterator++) {
            ss << printType(childNodesIterator.operator*(), ss_self.str());
        }

        return ss.str();
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

    StructureBiNode::StructureBiNode(BrowseResult_t browseResult,
                                         std::list<std::shared_ptr<const StructureNode>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex,
                                         ModellingRule_t modellingRule)
                                         : structureNode(std::make_shared<StructureNode>(browseResult, childNodes, modellingRule)),
                                         parent(parent),
                                         namespaceIndex(namespaceIndex)
                                         {}

    StructureBiNode::StructureBiNode(NodeClass_t nodeClass, ModellingRule_t modellingRule, NodeId_t referenceType,
                                         NodeId_t specifiedTypeNodeId, QualifiedName_t specifiedBrowseName,
                                         std::list<std::shared_ptr<const StructureNode>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex) :
            structureNode(std::make_shared<StructureNode>(nodeClass, modellingRule, referenceType, specifiedTypeNodeId, specifiedBrowseName,childNodes)),
            parent(parent),
            namespaceIndex(namespaceIndex)
            {}

    std::shared_ptr<StructureNode> StructureBiNode::toStructureNode() {

        if(this->SpecifiedBiChildNodes.size() > 0) {
            // todo transform BiChild to child;
            for(auto childIterator = this->SpecifiedBiChildNodes.begin(); childIterator != this->SpecifiedBiChildNodes.end(); childIterator++) {
                auto innerChild = childIterator.operator*();
                std::shared_ptr<StructureNode> innerChildStructureNode = innerChild->toStructureNode();
                this->structureNode->SpecifiedChildNodes.emplace_back(innerChildStructureNode);
            }

        }
        return this->structureNode;
    }
}
