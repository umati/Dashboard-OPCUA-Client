
#include <gtest/gtest.h>

#include <ConfigureLogger.hpp>
#include <OpcUaClient.hpp>
#include <algorithm>

#define OPCUA_TEST_SERVER_URL "opc.tcp://localhost:48010"
#define OPCUA_TEST_SERVER_NSURI "http://www.unifiedautomation.com/DemoServer/"

TEST(OpcUaClient, TranslateBrowsePathToNodeId)
{
	Umati::Util::ConfigureLogger("OpcUaClient.TranslateBrowsePathToNodeId");
	Umati::OpcUa::OpcUaClient client(OPCUA_TEST_SERVER_URL);
	ASSERT_TRUE(client.isConnected());

	ModelOpcUa::NodeId_t startNodeId{ "", "i=85" };
	//ModelOpcUa::NodeId_t startNodeId{ OPCUA_TEST_SERVER_NSURI, "s=Demo" };
	ModelOpcUa::QualifiedName_t browseNameDemo{ OPCUA_TEST_SERVER_NSURI, "Demo" };

	auto resultNodeId = client.TranslateBrowsePathToNodeId(startNodeId, browseNameDemo);
	ModelOpcUa::NodeId_t expectNodeId{ OPCUA_TEST_SERVER_NSURI, "s=Demo" };
	EXPECT_EQ(resultNodeId, expectNodeId);

}

TEST(OpcUaClient, Browse)
{
	Umati::Util::ConfigureLogger("OpcUaClient.Browse");
	Umati::OpcUa::OpcUaClient client(OPCUA_TEST_SERVER_URL);
	ASSERT_TRUE(client.isConnected());

	ModelOpcUa::NodeId_t startNodeId{ "", "i=85" };
	ModelOpcUa::NodeId_t folderTypeNodeId{ "", "i=61" };
	ModelOpcUa::NodeId_t hierarchicalReferenceTypeNodeId{ "", "i=33" };

	auto resultBrowse = client.Browse(startNodeId, hierarchicalReferenceTypeNodeId, folderTypeNodeId);

	EXPECT_EQ(resultBrowse.size(), 3);

	ModelOpcUa::QualifiedName_t BuildingAutomation{ "urn:UnifiedAutomation:CppDemoServer:BuildingAutomation", "BuildingAutomation" };
	ModelOpcUa::QualifiedName_t Demo{ "http://www.unifiedautomation.com/DemoServer/", "Demo" };
	ModelOpcUa::QualifiedName_t DemoXml{ "urn:UnifiedAutomation:CppDemoServer:UANodeSetXmlImport", "Demo" };
	std::list<ModelOpcUa::QualifiedName_t> ExpectedBrowseNames{ BuildingAutomation, Demo, DemoXml };
	for (auto &browseEntry : resultBrowse)
	{
		EXPECT_NE(
			std::find(ExpectedBrowseNames.begin(), ExpectedBrowseNames.end(), browseEntry.BrowseName),
			ExpectedBrowseNames.end()
		);

	}

}

