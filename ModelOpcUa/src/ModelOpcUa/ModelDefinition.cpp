 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include "ModelDefinition.hpp"
#include <cassert>
#include <utility>
#include <utility>

namespace ModelOpcUa {

	NodeDefinition::NodeDefinition(
			NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			QualifiedName_t specifiedBrowseName)
			: NodeClass(nodeClass),
			  ModellingRule(modellingRule),
			  ReferenceType(std::move(std::move(referenceType))),
			  SpecifiedTypeNodeId(std::move(std::move(specifiedTypeNodeId))),
			  SpecifiedBrowseName(std::move(std::move(specifiedBrowseName))) {

	}

	StructureNode::StructureNode(
			NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			QualifiedName_t specifiedBrowseName, bool ofBaseDataVariableType,
			std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes
	)
			:
			NodeDefinition(
					nodeClass,
					modellingRule,
					std::move(referenceType),
					std::move(specifiedTypeNodeId),
					std::move(specifiedBrowseName)
			),
			SpecifiedChildNodes(std::move(std::move(childNodes))) {
		this->ofBaseDataVariableType = ofBaseDataVariableType;
	}

	StructureNode::StructureNode(const ModelOpcUa::BrowseResult_t &browseResult, bool ofBaseDataVariableType,
								 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes) :
			SpecifiedChildNodes(std::move(std::move(childNodes))),
			NodeDefinition(browseResult.NodeClass, Optional, browseResult.ReferenceTypeId, browseResult.TypeDefinition,
						   browseResult.BrowseName) {
		this->ofBaseDataVariableType = ofBaseDataVariableType;
	}

	StructureNode::StructureNode(const ModelOpcUa::BrowseResult_t &browseResult,
								 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
								 ModellingRule_t modellingRule,
								 bool ofBaseDataVariableType) :
			SpecifiedChildNodes(std::move(std::move(childNodes))),
			NodeDefinition(browseResult.NodeClass, modellingRule, browseResult.ReferenceTypeId,
						   browseResult.TypeDefinition, browseResult.BrowseName) {
		this->ofBaseDataVariableType = ofBaseDataVariableType;
	}

	std::string StructureNode::printType(const std::shared_ptr<StructureNode> &node, const std::string &parentTree) {
		std::stringstream ss;
		std::stringstream ss_self;

		std::string modellingRule;
		std::string nodeClass;
		if (node->ModellingRule == ModellingRule_t::Optional) { modellingRule = "Optional"; }
		else if (node->ModellingRule == ModellingRule_t::Mandatory) { modellingRule = "Mandatory"; }
		else if (node->ModellingRule ==
				 ModellingRule_t::MandatoryPlaceholder) { modellingRule = "MandatoryPlaceholder"; }
		else if (node->ModellingRule == ModellingRule_t::OptionalPlaceholder) { modellingRule = "OptionalPlaceholder"; }

		if (node->NodeClass == NodeClass_t::Object) {
			nodeClass = "Object";
		} else if (node->NodeClass == NodeClass_t::Variable) {
			nodeClass = "Variable";
		} else if (node->NodeClass == NodeClass_t::Method) {
			nodeClass = "Method";
		} else if (node->NodeClass == NodeClass_t::ObjectType) {
			nodeClass = "ObjectType";
		} else if (node->NodeClass == NodeClass_t::VariableType) {
			nodeClass = "VariableType";
		} else if (node->NodeClass == NodeClass_t::ReferenceType) {
			nodeClass = "ReferenceType";
		} else if (node->NodeClass == NodeClass_t::DataType) {
			nodeClass = "DataType";
		} else if (node->NodeClass == NodeClass_t::View) {
			nodeClass = "View";
		}

		ss_self << parentTree << "->" << node->SpecifiedBrowseName.Name << " (" << nodeClass
				<< (modellingRule.empty() ? "" : "|") << modellingRule << ")";
		ss << ss_self.str() << std::endl;


		for (auto childNodesIterator = node->SpecifiedChildNodes->begin();
			 childNodesIterator != node->SpecifiedChildNodes->end(); childNodesIterator++) {
			ss << printType(childNodesIterator.operator*(), ss_self.str());
		}

		return ss.str();
	}

	void 
	StructureNode::printYamlIntern(const std::shared_ptr<StructureNode> &node, const std::string &parentTree,
								   int tabs, std::ostream &ss) {
		std::stringstream ss_self;
		std::stringstream tabsstream;
		for (int i = 0; i < tabs; i++) {
			tabsstream << "    ";
		}
		std::string tabsString = tabsstream.str();
		tabsstream << "    ";
		std::string tabStringPlus2 = tabsstream.str();

		ss_self << tabsString << node->SpecifiedBrowseName.Name << ":" << std::endl;
		ss_self << tabStringPlus2 << "ModellingRule: " << ModelOpcUa::ModellingRuleToString(node->ModellingRule) << std::endl;
		ss_self << tabStringPlus2 << "NodeClass: " << ModelOpcUa::NodeClassToString(node->NodeClass) << std::endl;
		ss_self << tabStringPlus2 << "ReferenceType: " << static_cast<std::string>(node->ReferenceType) << std::endl;
		ss_self << tabStringPlus2 << "TypeDefinition: " << static_cast<std::string>(node->SpecifiedTypeNodeId) << std::endl;
		ss_self << tabStringPlus2 << "OfBaseDataVariableType: " << node->ofBaseDataVariableType << std::endl;


		if (node->SpecifiedChildNodes->empty()) {
			ss << ss_self.str();
		} else {
			ss << ss_self.str();
			ss << tabStringPlus2 << "Children:" << std::endl;

			for (auto childNodesIterator = node->SpecifiedChildNodes->begin();
				 childNodesIterator != node->SpecifiedChildNodes->end(); childNodesIterator++) {

				printYamlIntern(childNodesIterator.operator*(), ss_self.str(), tabs + 2, ss);
				if ((childNodesIterator != node->SpecifiedChildNodes->end()) &&
					(childNodesIterator != --node->SpecifiedChildNodes->end())) {
				}
			}
		}
	}

	StructureNode::StructureNode(StructureNode *structureNode, bool ofBaseDataVariableType,
								 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes)
			:
			NodeDefinition(
					structureNode->NodeClass,
					structureNode->ModellingRule,
					structureNode->ReferenceType,
					structureNode->SpecifiedTypeNodeId,
					structureNode->SpecifiedBrowseName
			),
			SpecifiedChildNodes(std::move(std::move(childNodes))) {
		this->ofBaseDataVariableType = ofBaseDataVariableType;
	}

	StructurePlaceholderNode::StructurePlaceholderNode(
			NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			QualifiedName_t specifiedBrowseName,
			bool ofBaseDataVariableType,
			std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
			std::list<std::shared_ptr<const StructureNode>> possibleTypes

	)
			:
			StructureNode(
					nodeClass,
					modellingRule,
					std::move(referenceType),
					std::move(specifiedTypeNodeId),
					std::move(specifiedBrowseName), ofBaseDataVariableType,
					std::move(childNodes)
			),
			PossibleTypes(std::move(std::move(possibleTypes))) {
		assert(modellingRule == ModellingRule_t::MandatoryPlaceholder ||
			   modellingRule == ModellingRule_t::OptionalPlaceholder);
	}


	StructurePlaceholderNode::StructurePlaceholderNode(const std::shared_ptr<StructureNode> &sharedPtr)
			: StructureNode(sharedPtr->NodeClass, sharedPtr->ModellingRule, sharedPtr->ReferenceType,
							sharedPtr->SpecifiedTypeNodeId, sharedPtr->SpecifiedBrowseName,
							sharedPtr->ofBaseDataVariableType, sharedPtr->SpecifiedChildNodes) {
	}

	StructureBiNode::StructureBiNode(BrowseResult_t browseResult, bool ofBaseDataVariableType,
									 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
									 std::shared_ptr<StructureBiNode> parent, std::string namespaceUri)
			: structureNode(std::make_shared<StructureNode>(browseResult, false, childNodes)),
			  parent(std::move(std::move(parent))),
			  namespaceUri(namespaceUri) {}

	StructureBiNode::StructureBiNode(BrowseResult_t browseResult, bool ofBaseDataVariableType,
									 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
									 std::shared_ptr<StructureBiNode> parent, std::string namespaceUri,
									 ModellingRule_t modellingRule)
			: structureNode(
			std::make_shared<StructureNode>(browseResult, childNodes, modellingRule, ofBaseDataVariableType)),
			  parent(std::move(std::move(parent))),
			  namespaceUri(namespaceUri) {}

	StructureBiNode::StructureBiNode(NodeClass_t nodeClass, ModellingRule_t modellingRule, NodeId_t referenceType,
									 NodeId_t specifiedTypeNodeId, QualifiedName_t specifiedBrowseName,
									 bool ofBaseDataVariableType,
									 std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
									 std::shared_ptr<StructureBiNode> parent, std::string namespaceUri) :
			structureNode(std::make_shared<StructureNode>(nodeClass, modellingRule, referenceType, specifiedTypeNodeId,
														  specifiedBrowseName, ofBaseDataVariableType, childNodes)),
			parent(std::move(std::move(parent))),
			namespaceUri(namespaceUri) {}

	std::shared_ptr<StructureNode> StructureBiNode::toStructureNode() {

		if (!this->SpecifiedBiChildNodes->empty()) {
			for (auto childIterator = this->SpecifiedBiChildNodes->begin();
				 childIterator != this->SpecifiedBiChildNodes->end(); childIterator++) {
				auto innerChild = childIterator.operator*();
				std::shared_ptr<StructureNode> innerChildStructureNode = innerChild->toStructureNode();
				this->structureNode->SpecifiedChildNodes->emplace_back(innerChildStructureNode);
			}
		}
		this->structureNode->ofBaseDataVariableType = ofBaseDataVariableType;
		return this->structureNode;
	}
}
