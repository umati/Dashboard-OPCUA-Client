#pragma once

#include <IDashboardDataClient.hpp>

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <Open62541Cpp/UA_NodeId.hpp>
#include <Open62541Cpp/UA_String.hpp>
#include <Open62541Cpp/UA_QualifiedName.hpp>
#include <Open62541Cpp/UA_Variant.hpp>
#include <string>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include "ModelOpcUa/ModelDefinition.hpp"
#include "ModelOpcUa/ModelInstance.hpp"

#include "Subscription.hpp"
#include "OpcUaInterface.hpp"
#include <functional>

namespace Umati
{
	
	namespace OpcUa
	{
		class OpcUaClient : public Dashboard::IDashboardDataClient
		{
		//VERIFY 
		//UA_DISABLE_COPY(OpcUaClient);
		public:
			explicit OpcUaClient(std::string serverURI, std::string Username = std::string(),
								 std::string Password = std::string(), std::uint8_t security = 1,
								 std::vector<std::string> expectedObjectTypeNamespaces = std::vector<std::string>(),
								 std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>());
			~OpcUaClient() ;

			bool disconnect();
			bool isConnected() { return m_isConnected; }

			// Inherit from IDashboardClient
			std::list<ModelOpcUa::BrowseResult_t> Browse(
				ModelOpcUa::NodeId_t startNode,
				BrowseContext_t browseContext) override;

			std::list<ModelOpcUa::BrowseResult_t>
			BrowseWithResultTypeFilter(
				ModelOpcUa::NodeId_t startNode,
				BrowseContext_t browseContext,
				ModelOpcUa::NodeId_t typeDefinition) override;

			ModelOpcUa::NodeId_t
			TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode,
												 ModelOpcUa::QualifiedName_t browseName) override;

			std::shared_ptr<ValueSubscriptionHandle>
			Subscribe(UA_Client *client, ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) override;

			std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds) override;

			std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &_nodeId) override;

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) override;

			std::vector<std::string> Namespaces() override;

        protected:
			//FIXME override gives "marked ‘override’, but does not override" error
			void connectionStatusChanged(UA_Int32 clientConnectionId, UA_ServerState serverStatus);// override;
			
			bool connect();

			UA_NodeClass readNodeClass(const open62541Cpp::UA_NodeId &nodeId);

			void checkConnection();

			open62541Cpp::UA_NodeId browseSuperType(const open62541Cpp::UA_NodeId &typeNodeId);

			// Max search depth
			bool isSameOrSubtype(const open62541Cpp::UA_NodeId &expectedType, const open62541Cpp::UA_NodeId &checkType, std::size_t maxDepth = 100);
			
			double m_maxAgeRead_ms = 100.0;

			void updateNamespaceCache();

			void threadConnectExecution();

			std::shared_ptr<UA_SessionState> m_pSession;
			std::map<uint16_t, std::string> m_indexToUriCache;
			std::string m_serverUri;
			std::string m_username;
			std::string m_password;
			UA_MessageSecurityMode m_security = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;//UA_MESSAGESECURITYMODE_NONE;

			std::shared_ptr<std::thread> m_connectThread;
			std::shared_ptr<OpcUaInterface> m_opcUaWrapper;
			std::atomic_bool m_isConnected = {false};
			std::atomic_bool m_tryConnecting = {false};

			Subscription m_subscr;

			struct UaNodeId_Compare
			{
				bool operator()(const open62541Cpp::UA_NodeId &left, const open62541Cpp::UA_NodeId &right) const
				{	
						   return left < right;
				}
			};

			/// Map for chaching super types. Key = Type, Value = Supertype
			std::map<open62541Cpp::UA_NodeId, open62541Cpp::UA_NodeId, UaNodeId_Compare> m_superTypes;
		
        public:
            UA_Client *client;  // Zugriff aus dem ConnectThread, dem PublisherThread
            std::recursive_mutex m_clientMutex;
private:
			void on_connected();

			std::vector<UA_DataValue> readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds);

			UA_ApplicationDescription &
			prepareSessionConnectInfo(UA_ApplicationDescription &sessionConnectInfo);
			void initializeNamespaceCache();

			UA_BrowseDescription prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId);

			static void handleContinuationPoint(const UA_ByteString & /*continuationPoint*/);

			void ReferenceDescriptionsToBrowseResults(const std::vector<UA_ReferenceDescription> &referenceDescriptions,
			std::list<ModelOpcUa::BrowseResult_t> &browseResult,
			std::function<bool(const UA_ReferenceDescription &)> filter = [] (const UA_ReferenceDescription&) {return true;});

			std::list<ModelOpcUa::BrowseResult_t>
			BrowseWithContextAndFilter(
				const ModelOpcUa::NodeId_t &startNode,
				UA_BrowseDescription &browseContext,
				std::function<bool(const UA_ReferenceDescription&)> filter = [] (const UA_ReferenceDescription&) {return true;});

			static Umati::Dashboard::IDashboardDataClient::BrowseContext_t prepareObjectAndVariableTypeBrowseContext();
			UA_BrowseDescription getUaBrowseContext(const IDashboardDataClient::BrowseContext_t &browseContext);

			UA_NodeClass nodeClassFromNodeId(const open62541Cpp::UA_NodeId &typeDefinitionUaNodeId);

			ModelOpcUa::BrowseResult_t
			ReferenceDescriptionToBrowseResult(const UA_ReferenceDescription &referenceDescription);

			ModelOpcUa::ModellingRule_t browseModellingRule(const open62541Cpp::UA_NodeId &uaNodeId);

			static void
			updateResultContainer();

			void fillNamespaceCache(const std::vector<std::string> &uaNamespaces);
		};
	} // namespace OpcUa
} // namespace Umati
