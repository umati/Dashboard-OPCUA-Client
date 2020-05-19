
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

using ::testing::Return;
using ::testing::DoAll;
using ::testing::Expectation;
using ::testing::SetArgReferee;
using ::testing::Invoke;
using ::testing::AtLeast;

namespace Umati {
    namespace OpcUa {
        class TestOpcUaClient : public OpcUaClient {
        public:
            TestOpcUaClient(std::string serverURI, std::string Username = std::string(), std::string Password = std::string(), std::uint8_t security = 1, std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>())
            : OpcUaClient(serverURI, Username, Password, security, std::vector<std::string>(), opcUaWrapper) {};
            void callConnectionStatusChanged(UaClientSdk::UaClient::ServerStatus serverStatus) {
                OpcUa_UInt32 opcConnID = 1;
                connectionStatusChanged(opcConnID, serverStatus);
            };

            void callUpdateNamespaceCache() {
                updateNamespaceCache();
            }

            static std::shared_ptr<Umati::OpcUa::MockOpcUaWrapper>  getWrapper (){
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
                UaString endpointUrl ="opc.tcp://localhost:48010";
                OpcUa_String* opcEndpointUrl = endpointUrl.copy();
                description_a->SecurityMode = securityMode;
                description_a->EndpointUrl = *opcEndpointUrl;
                opcUaEndpointDescriptions[0] = *description_a;
                endpointDescriptions.setEndpointDescriptions(length, opcUaEndpointDescriptions);

                EXPECT_CALL(*mockWrapper, DiscoveryGetEndpoints).Times(AtLeast(1))
                        .WillRepeatedly(
                                DoAll(
                                        SetArgReferee<3>(endpointDescriptions),
                                        Return(uaStatus1)
                                )
                        );

                EXPECT_CALL(*mockWrapper, SessionConnect).Times(AtLeast(1)).WillRepeatedly(Return(uaStatus2));
                EXPECT_CALL(*mockWrapper, GetNewSession).Times(AtLeast(1));
                return mockWrapper;
            }
        };
    }
}


TEST(OpcUaClient, TranslateBrowsePathToNodeId)
{
	Umati::Util::ConfigureLogger("OpcUaClient.TranslateBrowsePathToNodeId");
    std::shared_ptr<Umati::OpcUa::MockOpcUaWrapper> mockWrapper = Umati::OpcUa::TestOpcUaClient::getWrapper();

    Umati::OpcUa::TestOpcUaClient client(std::string("someUrl"),std::string("someUser"), std::string("somePassword"), OpcUa_MessageSecurityMode_None, mockWrapper);
	client.callConnectionStatusChanged(UaClientSdk::UaClient::Connected);
    sleep(2);
	ASSERT_TRUE(client.isConnected());
}

TEST(OpcUaClient, updateNamespaceCache_updates)
{
	Umati::Util::ConfigureLogger("OpcUaClient.TranslateBrowsePathToNodeId");
    std::shared_ptr<Umati::OpcUa::MockOpcUaWrapper> mockWrapper = Umati::OpcUa::TestOpcUaClient::getWrapper();
    UaString endpointUrl1 = "opc.tcp://192.168.0.2:48010";
    UaString endpointUrl2 = "opc.tcp://192.168.0.20:48010";
    UaString endpointUrl3 = "opc.tcp://192.168.0.200:48010";
    OpcUa_String* opcEndpointUrl1 = endpointUrl1.copy();
    OpcUa_String* opcEndpointUrl2 = endpointUrl2.copy();
    OpcUa_String* opcEndpointUrl3 = endpointUrl3.copy();
    OpcUa_String array1[] = {*opcEndpointUrl1};
    OpcUa_String array2[] = {*opcEndpointUrl1, *opcEndpointUrl2, *opcEndpointUrl3};
    OpcUa_String array3[] = {*opcEndpointUrl2, *opcEndpointUrl3};

    UaStringArray uaNamespaces1(1, reinterpret_cast<OpcUa_String *>(array1));
    UaStringArray uaNamespaces2(3, reinterpret_cast<OpcUa_String *>(array2));
    UaStringArray uaNamespaces3(2, reinterpret_cast<OpcUa_String *>(array3));
    EXPECT_CALL(*mockWrapper, SessionGetNamespaceTable).Times(AtLeast(2))
    .WillOnce(Return(uaNamespaces1))
    .WillOnce(Return(uaNamespaces2))
    .WillRepeatedly(Return(uaNamespaces3));
    Umati::OpcUa::TestOpcUaClient client(std::string("someUrl"),std::string("someUser"), std::string("somePassword"), OpcUa_MessageSecurityMode_None, mockWrapper);
    client.callUpdateNamespaceCache();
    client.callUpdateNamespaceCache();
}

TEST(OpcUaClient, Browse)
{
	Umati::Util::ConfigureLogger("OpcUaClient.Browse");
    std::shared_ptr<Umati::OpcUa::MockOpcUaWrapper> mockWrapper = Umati::OpcUa::TestOpcUaClient::getWrapper();
    UaStatus good(OpcUa_Good);
    UaDataValue value((OpcUa_Int32) OpcUa_NodeClass_ObjectType, OpcUa_Good, NULL, NULL);
    OpcUa_DataValue* opcValue = value.copy();
    OpcUa_DataValue values[1];
    values[0] = *opcValue;
    UaDataValues readResult;
    readResult.setDataValues((OpcUa_Int32) 1, values);

    OpcUa_ReferenceDescription opcUaReferenceDescriptions[1] {
        OpcUa_ReferenceDescription()
    };


    UaReferenceDescriptions referenceDescriptions(1, opcUaReferenceDescriptions);

    EXPECT_CALL(*mockWrapper, SessionRead).Times(AtLeast(1)).WillRepeatedly(
            DoAll(
                    SetArgReferee<4>(readResult),
                    Return(good)
            )
        );
    EXPECT_CALL(*mockWrapper, SessionBrowse).Times(AtLeast(1)).WillRepeatedly(
    DoAll(
            SetArgReferee<4>(referenceDescriptions),
            Return(good)
    )
    );
    EXPECT_CALL(*mockWrapper, SessionIsConnected).Times(AtLeast(1)).WillRepeatedly(Return(true));

    Umati::OpcUa::TestOpcUaClient client(std::string("someUrl"),std::string("someUser"), std::string("somePassword"), OpcUa_MessageSecurityMode_None, mockWrapper);
    client.callConnectionStatusChanged(UaClientSdk::UaClient::Connected);

	ModelOpcUa::NodeId_t startNodeId{ "", "i=85" };
	ModelOpcUa::NodeId_t folderTypeNodeId{ "", "i=61" };
	ModelOpcUa::NodeId_t hierarchicalReferenceTypeNodeId{ "", "i=33" };

	auto resultBrowse = client.Browse(startNodeId, hierarchicalReferenceTypeNodeId, folderTypeNodeId);

/*	EXPECT_EQ(resultBrowse.size(), 3);

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

	}*/

}
/*
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

*/