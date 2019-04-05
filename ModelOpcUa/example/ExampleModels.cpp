
#include <string>

#include <ModelOpcUa/ModelOpcUa.hpp>
#include "ExampleModels.hpp"

namespace ExampleModels
{
	const ModelOpcUa::NodeId_t hasComponent{ "", "hasComponent" };
	const ModelOpcUa::NodeId_t BaseDataVariable {"", "BaseDataVariable" };
	const ModelOpcUa::NodeId_t BaseObjectType {"", "BaseObjectType" };
	const decltype(ModelOpcUa::NodeId_t::Uri) MyTypeNs("MyTypeNS");

	static std::shared_ptr<ModelOpcUa::StructureNode> getSimpleVariable(
		std::string name,
		ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::Mandatory
	)
	{
		return std::make_shared<ModelOpcUa::StructureNode>(
			ModelOpcUa::NodeClass_t::Variable,
			modellingRule,
			hasComponent,
			BaseDataVariable,
			ModelOpcUa::QualifiedName_t{ MyTypeNs, name }
			);
	}

	static std::shared_ptr<ModelOpcUa::StructurePlaceholderNode> getSimplePlaceholder(
		std::string name,
		std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> possibleTypes,
		ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::MandatoryPlaceholder
	)
	{
		return std::shared_ptr<ModelOpcUa::StructurePlaceholderNode>(new ModelOpcUa::StructurePlaceholderNode(
			ModelOpcUa::NodeClass_t::Variable,
			modellingRule,
			hasComponent,
			BaseDataVariable,
			{MyTypeNs, name },
			{},
			possibleTypes
		));
	}

	static std::list<std::shared_ptr<const ModelOpcUa::StructureNode>> ListOfNodes(
		std::initializer_list<std::shared_ptr<ModelOpcUa::StructureNode>> list)
	{
		return std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>(list.begin(), list.end());
	}

	std::shared_ptr<ModelOpcUa::StructureNode> getSimpleObject()
	{
		auto a = getSimpleVariable("a");
		auto b = getSimpleVariable("b");
		auto c = getSimpleVariable("c");

		std::initializer_list<std::shared_ptr<ModelOpcUa::StructureNode>> childs = { a,b,c };

		auto obj = std::make_shared<ModelOpcUa::StructureNode>(
			ModelOpcUa::NodeClass_t::Object,
			ModelOpcUa::ModellingRule_t::Mandatory,
			ModelOpcUa::NodeId_t {},
			ModelOpcUa::NodeId_t { MyTypeNs, "MyObjectType" },
			ModelOpcUa::QualifiedName_t {},
			ListOfNodes({ a, b, c })
			);

		return obj;
	}

	std::shared_ptr<ModelOpcUa::StructureNode> getSimpleObjectWithPlaceholder()
	{
		auto a = getSimpleVariable("a");
		auto b = getSimpleVariable("b");

		auto posObj = std::make_shared<ModelOpcUa::StructureNode>(
			ModelOpcUa::NodeClass_t::Object,
			ModelOpcUa::ModellingRule_t::Mandatory,
			ModelOpcUa::NodeId_t{},
			ModelOpcUa::NodeId_t{ MyTypeNs, "MyObjectType" },
			ModelOpcUa::QualifiedName_t{}
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
			ModelOpcUa::NodeId_t{ MyTypeNs, "MyObjectWithPlaceholderType" },
			ModelOpcUa::QualifiedName_t{},
			ListOfNodes({ a,b,c })
			);

		return obj;
	}
}
