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

std::string toString(const std::shared_ptr<const ModelOpcUa::StructureNode> &pStrucNode, int depth = 0);

///\todo avoid code duplicant
std::string toString(const std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> &pPlaceholderStrucNode, int depth = 0)
{
	if (!pPlaceholderStrucNode)
	{
		return "NULL";
	}
	std::string preLine1(depth * 2, ' ');
	std::string preLine(depth * 2 + 2, ' ');
	std::stringstream ss;
	ss << preLine1 << typeid(ModelOpcUa::StructurePlaceholderNode).name() << std::endl;
	ss << preLine << "NodeClass: " << toString(pPlaceholderStrucNode->NodeClass) << std::endl;
	ss << preLine << "SpecifiedBrowseName: " << static_cast<std::string>(pPlaceholderStrucNode->SpecifiedBrowseName) << std::endl;
	ss << preLine << "ReferenceType: " << static_cast<std::string>(pPlaceholderStrucNode->ReferenceType) << std::endl;

	ss << preLine << "possibleTypes(" << pPlaceholderStrucNode->PossibleTypes.size() << "): [" << std::endl;
	for (auto &pPossibleTypes : pPlaceholderStrucNode->PossibleTypes)
	{
		ss << toString(pPossibleTypes, depth + 1);
	}
	ss << preLine << "]" << std::endl;

	ss << preLine << "childs(" << pPlaceholderStrucNode->SpecifiedChildNodes.size() << "): [" << std::endl;
	for (auto & pChild : pPlaceholderStrucNode->SpecifiedChildNodes)
	{
		///\TODO use Visitor
		if (auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::StructurePlaceholderNode>(pChild))
		{
			ss << toString(pPlaceholderChild, depth + 1);
		}
		else
		{
			ss << toString(pChild, depth + 1);
		}
	}
	ss << preLine << "]" << std::endl;

	return ss.str();
}

std::string toString(const std::shared_ptr<const ModelOpcUa::StructureNode> &pStrucNode, int depth)
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
	ss << preLine << "SpecifiedBrowseName: " << static_cast<std::string>(pStrucNode->SpecifiedBrowseName) << std::endl;
	ss << preLine << "ReferenceType: " << static_cast<std::string>(pStrucNode->ReferenceType) << std::endl;
	ss << preLine << "childs(" << pStrucNode->SpecifiedChildNodes.size() << "): [" << std::endl;

	for (auto & pChild : pStrucNode->SpecifiedChildNodes)
	{
		///\TODO use Visitor
		if (auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::StructurePlaceholderNode>(pChild))
		{
			ss << toString(pPlaceholderChild, depth + 1);
		}
		else
		{
			ss << toString(pChild, depth + 1);
		}
	}
	ss << preLine << "]" << std::endl;

	return ss.str();
}

std::string toString(const std::shared_ptr<const ModelOpcUa::Node> &pNode, int depth = 0)
{
	if (!pNode)
	{
		return "NULL";
	}
	std::string preLine1(depth * 2, ' ');
	std::string preLine(depth * 2 + 2, ' ');
	std::stringstream ss;
	ss << preLine1 << typeid(ModelOpcUa::Node).name() << std::endl;
	ss << preLine << "NodeClass: " << toString(pNode->NodeClass) << std::endl;
	ss << preLine << "SpecifiedBrowseName: " << static_cast<std::string>(pNode->SpecifiedBrowseName) << std::endl;

	if (auto pSimpleNode = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pNode))
	{
		ss << preLine << "C++ Type: Simple Node" << std::endl;
		ss << preLine << "NodeId: " << static_cast<std::string>(pSimpleNode->NodeId) << std::endl;
	}
	else if (auto pSimpleNode = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pNode))
	{
		ss << preLine << "C++ Type: Placeholder Node" << std::endl;
	}

	//ss << preLine << "ReferenceType: " << pStrucNode->ReferenceType << std::endl;
	ss << preLine << "childs(" << pNode->ChildNodes.size() << "): [" << std::endl;

	for (auto & pChild : pNode->ChildNodes)
	{
		ss << toString(pChild, depth + 1);
	}
	ss << preLine << "]" << std::endl;

	return ss.str();
}

ModelOpcUa::NodeId_t translateBrowsePathToNodeIdMock(ModelOpcUa::NodeId_t startNode, ModelOpcUa::QualifiedName_t BrowseName)
{
	ModelOpcUa::NodeId_t ret;
	if (!startNode.isNull())
	{
		ret.Uri = startNode.Uri;
		ret.Id = startNode.Id + "." + BrowseName.Name;
	}

	return ret;
}

struct BrowseResult_t {
    ModelOpcUa::NodeId_t NodeId;
    ModelOpcUa::NodeId_t TypeDefinitionId;
    std::string BrowseName;
    std::string NodeClass;
};


std::list<BrowseResult_t> browseNodes(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId, ModelOpcUa::NodeId_t nodeTypeId)
{
	return std::list<BrowseResult_t>();
}

std::shared_ptr<const ModelOpcUa::PlaceholderNode> browsePlaceholder(
	ModelOpcUa::NodeId_t startNode,
	std::shared_ptr<const ModelOpcUa::StructurePlaceholderNode> pStrucPlaceholder)
{
	if (pStrucPlaceholder)
	{
		return nullptr;
	}
	pStrucPlaceholder->ReferenceType;

	auto browseResults = browseNodes(startNode, pStrucPlaceholder->ReferenceType, pStrucPlaceholder->SpecifiedTypeNodeId);
	for (auto &browseResult : browseResults)
	{
		// Look for the correct type in possibleTypes based on the browseType
		// Create appropriate instance node

	}
}

std::shared_ptr<const ModelOpcUa::SimpleNode> transformToNodeIds(
	const std::shared_ptr<const ModelOpcUa::StructureNode> &pStrucNode,
	ModelOpcUa::NodeId_t startNode
)
{
	std::list<std::shared_ptr<const ModelOpcUa::Node>> foundChildNodes;
	for (auto & pChild : pStrucNode->SpecifiedChildNodes)
	{
		switch (pChild->ModellingRule)
		{
		case ModelOpcUa::ModellingRule_t::Mandatory:
		case ModelOpcUa::ModellingRule_t::Optional:
		{
			auto childNodeId = translateBrowsePathToNodeIdMock(startNode, pChild->SpecifiedBrowseName);
			if (childNodeId.isNull())
			{
				continue;
			}
			foundChildNodes.push_back(transformToNodeIds(pChild, childNodeId));

			break;
		}
		case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
		case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
		{
			auto pPlaceholderChild = std::dynamic_pointer_cast<const ModelOpcUa::StructurePlaceholderNode>(pChild);
			if (!pPlaceholderChild)
			{
				std::cout << "Placeholder error." << std::endl;
				break;
			}
			auto childNodes = browsePlaceholder(startNode, pPlaceholderChild);
			std::cout << "Placeholder not supported." << std::endl;
			break;
		}
		default:
			std::cout << "Unknown Modelling Rule." << std::endl;
			break;
		}
	}

	auto pNode = std::make_shared<ModelOpcUa::SimpleNode>(
		startNode,
		ModelOpcUa::NodeId_t {"AnyNs", "TODO Set Type" },
		*pStrucNode,
		foundChildNodes
	);
	return pNode;
}

TEST(BasicUsage, Simple)
{
	auto obj = ExampleModels::getSimpleObject();
	std::cout << toString(obj) << std::endl;

	EXPECT_TRUE(true);
}

TEST(BasicUsage, SimpleTranslateToNodeId)
{
	auto node = transformToNodeIds(ExampleModels::getSimpleObject(), { "MyNS", "Example" });

	std::cout << toString(node) << std::endl;

	EXPECT_TRUE(true);
}

TEST(BasicUsage, SimplePlaceholder)
{
	auto obj = ExampleModels::getSimpleObjectWithPlaceholder();
	std::cout << toString(obj) << std::endl;

	EXPECT_TRUE(true);
}

TEST(BasicUsage, SimplePlaceholderTranslate)
{
	auto node = transformToNodeIds(ExampleModels::getSimpleObjectWithPlaceholder(), { "MyNS", "Example" });

	std::cout << toString(node) << std::endl;

	EXPECT_TRUE(true);
}
