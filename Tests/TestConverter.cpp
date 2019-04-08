#include <gtest/gtest.h>

#include <Converter/UaNodeIdToModelNodeId.hpp>
#include <Converter/ModelNodeIdToUaNodeId.hpp>
#include <Converter/ModelQualifiedNameToUaQualifiedName.hpp>

TEST(Converter, NodeId)
{
	ModelOpcUa::NodeId_t nodeId{ "MyURI", "s=StringId" };

	std::map<std::string, uint16_t> uri2Id{ {"MyURI", 2} };
	std::map<uint16_t, std::string> id2Uri{ {2, "MyURI"} };

	auto uaNodeId = Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, uri2Id).getNodeId();
	auto convNodeId = Umati::OpcUa::Converter::UaNodeIdToModelNodeId(uaNodeId, id2Uri).getNodeId();

	EXPECT_EQ(nodeId, convNodeId);
}

TEST(Converter, QualifiedName)
{
	ModelOpcUa::QualifiedName_t qualName{ "MyURI", "MyName" };

	std::map<std::string, uint16_t> uri2Id{ {"MyURI", 2} };
	std::map<uint16_t, std::string> id2Uri{ {2, "MyURI"} };

	auto uaQualName = Umati::OpcUa::Converter::ModelQualifiedNameToUaQualifiedName(qualName, uri2Id).getQualifiedName();
	
	EXPECT_EQ(uaQualName.namespaceIndex(), 2);
	EXPECT_EQ(UaString(uaQualName.name()).toUtf8(), qualName.Name);
}
