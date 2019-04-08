
#include <gtest/gtest.h>

#include <ConfigureLogger.hpp>
#include <OpcUaClient.hpp>

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
