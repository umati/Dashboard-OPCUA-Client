#include <ModelOpcUa/ModelOpcUa.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <type_traits>

std::string toString(ModelOpcUa::NodeClass_t nodeClass)
{
	switch (nodeClass)
	{
	case ModelOpcUa::Object:
		return "Object";
		break;
	case ModelOpcUa::Variable:
		return "Variable";
		break;
	default:
		return "Unknown";
		break;
	}
}

std::string toString(const std::shared_ptr<const ModelOpcUa::StructureNode> &pStrucNode, int depth = 0)
{
	if (!pStrucNode)
	{
		return "NULL";
	}
	std::string preLine1(depth * 2, ' ');
	std::string preLine(depth * 2 + 2, ' ');
	std::stringstream ss;
	ss << preLine1 << typeid(ModelOpcUa::StructureNode).name() << std::endl;
	ss << preLine << "NodeClass: " << toString(pStrucNode->NodeClass) << std::endl;
	ss << preLine << "SpecifiedBrowseName: " << pStrucNode->SpecifiedBrowseName << std::endl;
	ss << preLine << "ReferenceType: " << pStrucNode->ReferenceType << std::endl;
	ss << preLine << "childs(" << pStrucNode->ChildNodes.size() << "): [" << std::endl;

	for (auto & pChild : pStrucNode->ChildNodes)
	{
		ss << toString(pChild, depth + 1);
	}
	ss << preLine << "]" << std::endl;

	return ss.str();
}

TEST(BasicUsage, Simple)
{
	const std::string hasComponent("hasComponent");
	const std::string BaseDataVariable("BaseDataVariable");
	auto a = std::make_shared<ModelOpcUa::StructureNode>(
		ModelOpcUa::NodeClass_t::Variable,
		ModelOpcUa::ModellingRule_t::Mandatory,
		hasComponent,
		BaseDataVariable,
		"a"
		);

	auto b = std::make_shared<ModelOpcUa::StructureNode>(
		ModelOpcUa::NodeClass_t::Variable,
		ModelOpcUa::ModellingRule_t::Mandatory,
		hasComponent,
		BaseDataVariable,
		"b"
		);

	auto c = std::make_shared<ModelOpcUa::StructureNode>(
		ModelOpcUa::NodeClass_t::Variable,
		ModelOpcUa::ModellingRule_t::Mandatory,
		hasComponent,
		BaseDataVariable,
		"c"
		);

	/*std::initializer_list<std::shared_ptr<ModelOpcUa::SimpleNode>> childs = {
		std::static_pointer_cast<ModelOpcUa::SimpleNode>(a),
		std::static_pointer_cast<ModelOpcUa::SimpleNode>(b),
		std::static_pointer_cast<ModelOpcUa::SimpleNode>(c)
	};*/

	std::initializer_list<std::shared_ptr<ModelOpcUa::StructureNode>> childs = { a,b,c };

	std::shared_ptr<ModelOpcUa::StructureNode> obj(new ModelOpcUa::StructureNode(
		ModelOpcUa::NodeClass_t::Object,
		ModelOpcUa::ModellingRule_t::Mandatory,
		"",
		"MyObjectType",
		"",
		{ a,b,c }
	));

	std::cout << toString(obj) << std::endl;

	EXPECT_TRUE(true);
}
