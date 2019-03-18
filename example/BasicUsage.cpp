#include <ModelOpcUa/ModelOpcUa.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <type_traits>
#include "ExampleModels.hpp"

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
	auto obj = ExampleModels::getSimpleObject();
	std::cout << toString(obj) << std::endl;

	EXPECT_TRUE(true);
}
