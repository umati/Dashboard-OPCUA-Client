
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
        std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes
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
                                 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes):
            SpecifiedChildNodes(childNodes), NodeDefinition(browseResult.NodeClass, Optional, browseResult.ReferenceTypeId, browseResult.TypeDefinition, browseResult.BrowseName)
            {

            }

    StructureNode::StructureNode(ModelOpcUa::BrowseResult_t browseResult,
                                 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes, ModellingRule_t modellingRule):
            SpecifiedChildNodes(childNodes), NodeDefinition(browseResult.NodeClass, modellingRule, browseResult.ReferenceTypeId, browseResult.TypeDefinition, browseResult.BrowseName)
            {

            }

    std::string StructureNode::printType(const std::shared_ptr<StructureNode>& node, const std::string& parentTree) {
        std::stringstream ss;
        std::stringstream ss_self;

        std::string modellingRule;
        std::string nodeClass;
        if (node->ModellingRule == ModellingRule_t::Optional) {modellingRule = "Optional"; }
        else if (node->ModellingRule == ModellingRule_t::Mandatory) { modellingRule = "Mandatory";}
        else if (node->ModellingRule == ModellingRule_t::MandatoryPlaceholder) { modellingRule = "MandatoryPlaceholder"; }
        else if (node->ModellingRule == ModellingRule_t::OptionalPlaceholder) { modellingRule = "OptionalPlaceholder"; }

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

        ss_self << parentTree << "->" << node->SpecifiedBrowseName.Name << " (" << nodeClass << (modellingRule.empty() ? "" : "|" ) << modellingRule << ")";
        ss << ss_self.str() << std::endl;


        for (auto childNodesIterator = node->SpecifiedChildNodes->begin(); childNodesIterator != node->SpecifiedChildNodes->end(); childNodesIterator++) {
            ss << printType(childNodesIterator.operator*(), ss_self.str());
        }

        return ss.str();
    }

    std::string StructureNode::printJson(const std::shared_ptr<StructureNode>& node) {
        std::stringstream ss;
        ss << "{" << std::endl << printJsonIntern(node, "", 0) << "}" << std::endl;
        return ss.str();
    }

    std::string StructureNode::printJsonIntern(const std::shared_ptr<StructureNode>& node, const std::string& parentTree, int tabs) {
        std::stringstream ss_self;
        std::stringstream ss;
        std::stringstream tabsstream;
        for(int i = 0; i < tabs; i++) {
            tabsstream << "\t";
        }
        std::string tabsString = tabsstream.str();

        ss_self << tabsString << "\"" << node->SpecifiedBrowseName.Name << "\":";
        if(node->SpecifiedChildNodes->empty()) {
            ss << ss_self.str() <<  " null";
        } else {
            ss << ss_self.str() << std::endl;
            for (auto childNodesIterator = node->SpecifiedChildNodes->begin(); childNodesIterator != node->SpecifiedChildNodes->end(); childNodesIterator++) {
                if (childNodesIterator == node->SpecifiedChildNodes->begin()) {
                    ss << tabsString << "{" << std::endl;
                }
                ss << printJsonIntern(childNodesIterator.operator*(), ss_self.str(), tabs + 1);
                if ((childNodesIterator != node->SpecifiedChildNodes->end()) && (childNodesIterator != --node->SpecifiedChildNodes->end())) {
                    ss << "," << std::endl;
                } else {
                    ss << std::endl << tabsString << "}";
                }
            }
        }

        return ss.str();
    }

    StructureNode::StructureNode(StructureNode *structureNode,
                                 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes)
            :
            NodeDefinition(
                    structureNode->NodeClass,
                    structureNode->ModellingRule,
                    structureNode->ReferenceType,
                    structureNode->SpecifiedTypeNodeId,
                    structureNode->SpecifiedBrowseName
            ),
            SpecifiedChildNodes(childNodes) {
    }

    StructurePlaceholderNode::StructurePlaceholderNode(
		NodeClass_t nodeClass,
		ModellingRule_t modellingRule,
		NodeId_t referenceType,
		NodeId_t specifiedTypeNodeId,
		QualifiedName_t specifiedBrowseName,
        std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
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


    StructurePlaceholderNode::StructurePlaceholderNode(const std::shared_ptr<StructureNode> sharedPtr)
    : StructureNode(sharedPtr->NodeClass,sharedPtr->ModellingRule,sharedPtr->ReferenceType, sharedPtr->SpecifiedTypeNodeId,sharedPtr->SpecifiedBrowseName,sharedPtr->SpecifiedChildNodes) {
	}

    StructureBiNode::StructureBiNode(BrowseResult_t browseResult,
                                     std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex)
                                         : structureNode(std::make_shared<StructureNode>(browseResult, childNodes)),
                                         parent(parent),
                                         namespaceIndex(namespaceIndex)  {}

    StructureBiNode::StructureBiNode(BrowseResult_t browseResult,
                                     std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex,
                                         ModellingRule_t modellingRule)
                                         : structureNode(std::make_shared<StructureNode>(browseResult, childNodes, modellingRule)),
                                         parent(parent),
                                         namespaceIndex(namespaceIndex)
                                         {}

    StructureBiNode::StructureBiNode(NodeClass_t nodeClass, ModellingRule_t modellingRule, NodeId_t referenceType,
                                         NodeId_t specifiedTypeNodeId, QualifiedName_t specifiedBrowseName,
                                     std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
                                         std::shared_ptr<StructureBiNode> parent, uint16_t namespaceIndex) :
            structureNode(std::make_shared<StructureNode>(nodeClass, modellingRule, referenceType, specifiedTypeNodeId, specifiedBrowseName,childNodes)),
            parent(parent),
            namespaceIndex(namespaceIndex)
            {}

    std::shared_ptr<StructureNode> StructureBiNode::toStructureNode() {

        if(this->SpecifiedBiChildNodes->size() > 0) {
            for(auto childIterator = this->SpecifiedBiChildNodes->begin(); childIterator != this->SpecifiedBiChildNodes->end(); childIterator++) {
                auto innerChild = childIterator.operator*();
                std::shared_ptr<StructureNode> innerChildStructureNode = innerChild->toStructureNode();
                this->structureNode->SpecifiedChildNodes->emplace_back(innerChildStructureNode);
            }
        }
        return this->structureNode;
    }
}
