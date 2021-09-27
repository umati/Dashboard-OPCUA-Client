/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include <string>

#include <ModelOpcUa/ModelOpcUa.hpp>
#include <utility>
#include "ExampleModels.hpp"

namespace ExampleModels {
	const ModelOpcUa::NodeId_t HasComponentTypeNodeId{"", "hasComponent"};
	const ModelOpcUa::NodeId_t BaseDataVariableTypeNodeId{"", "BaseDataVariable"};
	const ModelOpcUa::NodeId_t BaseObjectTypeNodeId{"", "BaseObjectType"};
	const decltype(ModelOpcUa::NodeId_t::Uri) MyTypeNs("MyTypeNS");

	static std::shared_ptr<ModelOpcUa::StructureNode> getSimpleVariable(
			std::string name,
			ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::Mandatory
	) {
		return std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Variable,
				modellingRule,
				HasComponentTypeNodeId,
				BaseDataVariableTypeNodeId,
				ModelOpcUa::QualifiedName_t{MyTypeNs, std::move(name)},
				false
		);
	}

	static std::shared_ptr<ModelOpcUa::StructurePlaceholderNode> getSimplePlaceholder(
			std::string name,
			std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> possibleTypes,
			ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::MandatoryPlaceholder
	) {
		return std::shared_ptr<ModelOpcUa::StructurePlaceholderNode>(new ModelOpcUa::StructurePlaceholderNode(
				ModelOpcUa::NodeClass_t::Variable,
				modellingRule,
				HasComponentTypeNodeId,
				BaseDataVariableTypeNodeId,
				{MyTypeNs, std::move(name)},
				false,
				{},
				std::move(possibleTypes)
		));
	}

	static std::shared_ptr<std::list<std::shared_ptr<ModelOpcUa::StructureNode>>> ListOfNodes(
			std::initializer_list<std::shared_ptr<ModelOpcUa::StructureNode>> list) {
		return std::make_shared<std::list<std::shared_ptr<ModelOpcUa::StructureNode>>>(list.begin(), list.end());
	}

	std::shared_ptr<ModelOpcUa::StructureNode> getSimpleObject() {
		auto a = getSimpleVariable("a");
		auto b = getSimpleVariable("b");
		auto c = getSimpleVariable("c");

		std::initializer_list<std::shared_ptr<ModelOpcUa::StructureNode>> children = {a, b, c};

		auto obj = std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Object,
				ModelOpcUa::ModellingRule_t::Mandatory,
				ModelOpcUa::NodeId_t{},
				ModelOpcUa::NodeId_t{MyTypeNs, "MyObjectType"},
				ModelOpcUa::QualifiedName_t{},
				false,
				ListOfNodes({a, b, c})
		);

		return obj;
	}

	std::shared_ptr<ModelOpcUa::StructureNode> getSimpleObjectWithPlaceholder() {
		auto a = getSimpleVariable("a");
		auto b = getSimpleVariable("b");

		auto posObj = std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Object,
				ModelOpcUa::ModellingRule_t::Mandatory,
				ModelOpcUa::NodeId_t{},
				ModelOpcUa::NodeId_t{MyTypeNs, "MyObjectType"},
				ModelOpcUa::QualifiedName_t{},
				false
		);

		auto c = getSimplePlaceholder(
				"Placeholder",
				{
						posObj
				});

		auto obj = std::make_shared<ModelOpcUa::StructureNode>(
				ModelOpcUa::NodeClass_t::Object,
				ModelOpcUa::ModellingRule_t::Mandatory,
				ModelOpcUa::NodeId_t{},
				ModelOpcUa::NodeId_t{MyTypeNs, "MyObjectWithPlaceholderType"},
				ModelOpcUa::QualifiedName_t{},
				false,
				ListOfNodes({a, b, c})
		);

		return obj;
	}
}
