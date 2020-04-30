
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "uaclientsdk_mock.hpp"
#include "../../_install-Debug/include/gmock/gmock-spec-builders.h"

#include <ConfigureLogger.hpp>
#include <OpcUaClient.hpp>
#include <algorithm>
#include <uadiscovery.h>
#include <uasession.h>
#include <opcua_proxystub.h>
#include <uaplatformlayer.h>

#define OPCUA_TEST_SERVER_URL "opc.tcp://localhost:48010"
#define OPCUA_TEST_SERVER_NSURI "http://www.unifiedautomation.com/DemoServer/"

using ::testing::Return;
using ::testing::DoAll;
using ::testing::Expectation;
using ::testing::SetArgReferee;
using ::testing::Invoke;



TEST(OpcUaClient, TranslateBrowsePathToNodeId)
{
	Umati::Util::ConfigureLogger("OpcUaClient.TranslateBrowsePathToNodeId");
    std::shared_ptr<Umati::OpcUa::MockOpcUaWrapper> mockWrapper = std::make_shared<Umati::OpcUa::MockOpcUaWrapper>();
    UaPlatformLayer::init();

    UaStatus uaStatus1(OpcUa_Good);
    UaStatus uaStatus2(OpcUa_Good);
    UaEndpointDescriptions endpointDescriptions;
    int length = 1;
    OpcUa_MessageSecurityMode security = OpcUa_MessageSecurityMode_None;
    OpcUa_EndpointDescription opcUaEndpointDescriptions[length];
    auto description_a = new OpcUa_EndpointDescription();
    OpcUa_MessageSecurityMode securityMode = security;
    UaString endpointUrl ="someUrl";
    OpcUa_String* opcEndpointUrl;
    endpointUrl.copyTo(opcEndpointUrl);
    description_a->SecurityMode = securityMode;
    description_a->EndpointUrl = *opcEndpointUrl;
    opcUaEndpointDescriptions[0] = *description_a;
    endpointDescriptions.setEndpointDescriptions(length, opcUaEndpointDescriptions);

    EXPECT_CALL(*mockWrapper, GetEndpoints).Times(1)
	    .WillOnce(
	            DoAll(
                        SetArgReferee<3>(endpointDescriptions),
	                    Return(uaStatus1)
	        )
	    );

    EXPECT_CALL(*mockWrapper, SessionConnect).WillOnce(Return(uaStatus2));
    EXPECT_CALL(*mockWrapper, GetNewSession).Times(1);
	Umati::OpcUa::OpcUaClient client(std::string("someUrl"),std::string("someUser"), std::string("somePassword"), security, mockWrapper);
	ASSERT_TRUE(client.isConnected());

	ModelOpcUa::NodeId_t startNodeId{ "", "i=85" };
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

TEST(OpcUaClient, ReadValue)
{
	ModelOpcUa::NodeId_t scalarStringNodeId{ "http://www.unifiedautomation.com/DemoServer/", "s=Demo.Static.Scalar.String" };
	Umati::Util::ConfigureLogger("OpcUaClient.Read");
	Umati::OpcUa::OpcUaClient client(OPCUA_TEST_SERVER_URL);
	ASSERT_TRUE(client.isConnected());

	auto valueList = client.readValues(std::list<ModelOpcUa::NodeId_t>{scalarStringNodeId});

	ASSERT_EQ(1, valueList.size());

	EXPECT_EQ(valueList.front()["value"], "Hello world");
}

