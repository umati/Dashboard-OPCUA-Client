#pragma once

#include <IDashboardDataClient.hpp>

#include <uabase.h>
#include <uaclientsdk.h>
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
		class OpcUaClient : public UaClientSdk::UaSessionCallback, public Dashboard::IDashboardDataClient
		{
			UA_DISABLE_COPY(OpcUaClient);

		public:
			explicit OpcUaClient(std::string serverURI, std::string Username = std::string(),
								 std::string Password = std::string(), std::uint8_t security = 1,
								 std::vector<std::string> expectedObjectTypeNamespaces = std::vector<std::string>(),
								 std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>());

			~OpcUaClient() override;

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

			ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode,
															 ModelOpcUa::QualifiedName_t browseName) override;

			std::shared_ptr<ValueSubscriptionHandle>
			Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) override;

			std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> nodeIds) override;

			std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &nodeId) override;

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) override;

			void CreateMachineListForNamespaceUnderStartNode(std::list<ModelOpcUa::BrowseResult_t> &machineList,
															 const std::string &startNodeNamespaceUri,
															 const ModelOpcUa::NodeId_t &startNode) override;

			void
			FillIdentificationValuesFromBrowseResult(std::list<ModelOpcUa::BrowseResult_t> &identification,
													 std::list<ModelOpcUa::NodeId_t> &identificationNodes,
													 std::vector<std::string> &identificationValueKeys) override;

			std::vector<std::string> Namespaces() override;

		protected:
			void connectionStatusChanged(OpcUa_UInt32 clientConnectionId,
										 UaClientSdk::UaClient::ServerStatus serverStatus) override;

			bool connect();

			OpcUa_NodeClass readNodeClass(const UaNodeId &nodeId);

			void checkConnection();

			UaNodeId browseSuperType(const UaNodeId &typeNodeId);

			// Max search depth
			bool isSameOrSubtype(const UaNodeId &expectedType, const UaNodeId &checkType, std::size_t maxDepth = 100);

			// ------- Default call settings -----------
			UaClientSdk::ServiceSettings m_defaultServiceSettings;
			double m_maxAgeRead_ms = 100.0;

			void updateNamespaceCache();

			void threadConnectExecution();

			std::shared_ptr<UaClientSdk::UaSession> m_pSession;
			std::map<uint16_t, std::string> m_indexToUriCache;
			std::string m_serverUri;
			std::string m_username;
			std::string m_password;
			OpcUa_MessageSecurityMode m_security = OpcUa_MessageSecurityMode::OpcUa_MessageSecurityMode_None;

			std::shared_ptr<std::thread> m_connectThread;
			std::shared_ptr<OpcUaInterface> m_opcUaWrapper;
			std::atomic_bool m_isConnected = {false};
			std::atomic_bool m_tryConnecting = {false};

			Subscription m_subscr;

			struct UaNodeId_Compare
			{
				bool operator()(const UaNodeId &left, const UaNodeId &right) const
				{
					return std::string(left.toXmlString().toUtf8()) <
						   std::string(right.toXmlString().toUtf8());
				}
			};

			/// Map for chaching super types. Key = Type, Value = Supertype
			std::map<UaNodeId, UaNodeId, UaNodeId_Compare> m_superTypes;

		private:
			static int PlatformLayerInitialized;

			void on_connected();

			UaDataValues readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds);

			static UaClientSdk::SessionConnectInfo &
			prepareSessionConnectInfo(UaClientSdk::SessionConnectInfo &sessionConnectInfo);
			void initializeNamespaceCache();

			UaClientSdk::BrowseContext prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId);

			static void handleContinuationPoint(const UaByteString &continuationPoint);

			void ReferenceDescriptionsToBrowseResults(
				const UaReferenceDescriptions &referenceDescriptions,
				std::list<ModelOpcUa::BrowseResult_t> &browseResult,
				std::function<bool(const OpcUa_ReferenceDescription&)> filter = [] (const OpcUa_ReferenceDescription&) {return true;}
				);

			std::list<ModelOpcUa::BrowseResult_t>
			BrowseWithContextAndFilter(
				const ModelOpcUa::NodeId_t &startNode,
				UaClientSdk::BrowseContext &browseContext,
				std::function<bool(const OpcUa_ReferenceDescription&)> filter = [] (const OpcUa_ReferenceDescription&) {return true;});

			static Umati::Dashboard::IDashboardDataClient::BrowseContext_t prepareObjectAndVariableTypeBrowseContext();
			UaClientSdk::BrowseContext getUaBrowseContext(const BrowseContext_t &browseContext);

			void browseUnderStartNode(const UaNodeId &startUaNodeId, UaReferenceDescriptions &referenceDescriptions);

			void browseUnderStartNode(const UaNodeId &startUaNodeId, UaReferenceDescriptions &referenceDescriptions,
									  const UaClientSdk::BrowseContext &browseContext);

			OpcUa_NodeClass nodeClassFromNodeId(const UaNodeId &typeDefinitionUaNodeId);

			ModelOpcUa::BrowseResult_t
			ReferenceDescriptionToBrowseResult(const OpcUa_ReferenceDescription &referenceDescriptions);

			ModelOpcUa::ModellingRule_t browseModellingRule(const UaNodeId &uaNodeId);

			static void
			split(const std::string &inputString, std::vector<std::string> &resultContainer, char delimiter);

			static void
			updateResultContainer(const std::string &inputString, std::vector<std::string> &resultContainer,
								  size_t current_char_position, size_t previous_char_position);

			void fillNamespaceCache(const UaStringArray &uaNamespaces);
		};
	} // namespace OpcUa
} // namespace Umati
