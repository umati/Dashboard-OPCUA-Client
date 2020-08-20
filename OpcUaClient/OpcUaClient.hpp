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

namespace Umati {
	namespace OpcUa {
		class OpcUaClient : public UaClientSdk::UaSessionCallback, public Dashboard::IDashboardDataClient
		{
			UA_DISABLE_COPY(OpcUaClient);
		public:
			OpcUaClient(std::string serverURI, std::string Username = std::string(), std::string Password = std::string(), std::uint8_t security = 1, std::vector<std::string> expectedObjectTypeNamespaces = std::vector<std::string>(), std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>());
			~OpcUaClient();

			bool disconnect();


			bool isConnected() { return m_isConnected; }

			// Inherit from IDashboardClient
			virtual std::list<ModelOpcUa::BrowseResult_t> Browse(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId, ModelOpcUa::NodeId_t typeDefinition) override;
			virtual ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode, ModelOpcUa::QualifiedName_t browseName) override;
			virtual std::shared_ptr<ValueSubscriptionHandle> Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) override;
			virtual void browseUnderStartNode(UaNodeId startUaNodeId, UaReferenceDescriptions &referenceDescriptions) override;
            virtual void browseUnderStartNode(UaNodeId startUaNodeId,UaReferenceDescriptions &referenceDescriptions, UaClientSdk::BrowseContext browseContext) override;
            virtual ModelOpcUa::BrowseResult_t ReferenceDescriptionToBrowseResult(const OpcUa_ReferenceDescription &referenceDescriptions) override;
            std::vector<nlohmann::json> readValues(std::list< ModelOpcUa::NodeId_t> nodeIds) override;
            UaDataValues readValues2(std::list<ModelOpcUa::NodeId_t> modelNodeIds) override;
            std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &nodeId);
            std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId);
		protected:
			void connectionStatusChanged(OpcUa_UInt32 clientConnectionId, UaClientSdk::UaClient::ServerStatus serverStatus) override;

			bool connect();

			OpcUa_NodeClass readNodeClass(UaNodeId nodeId);

			void checkConnection();

			UaNodeId browseSuperType(UaNodeId typeNodeId);

			// Max search depth
			bool isSameOrSubtype(UaNodeId expectedType, UaNodeId checkType, std::size_t maxDepth = 100);

			// ------- Default call settings -----------
			UaClientSdk::ServiceSettings m_defaultServiceSettings;
			double m_maxAgeRead_ms = 100.0;
			OpcUa_UInt32 m_nextTransactionid = 0x80000000;
			// -----------------------------------------

			void updateNamespaceCache();

			void threadConnectExecution();

			std::shared_ptr<UaClientSdk::UaSession> m_pSession;
			std::map<uint16_t, std::string> m_indexToUriCache;
            std::string m_serverUri;
			std::string m_username;
			std::string m_password;
			std::vector<std::string> m_expectedObjectTypeNamespaces;
			OpcUa_MessageSecurityMode m_security = OpcUa_MessageSecurityMode::OpcUa_MessageSecurityMode_None;

			std::shared_ptr<std::thread> m_connectThread;
			std::shared_ptr<OpcUaInterface> m_opcUaWrapper;
			std::atomic_bool m_isConnected = { false };
			std::atomic_bool m_tryConnecting = { false };

			Subscription m_subscr;

			struct UaNodeId_Compare
			{
				bool operator()(const UaNodeId & left, const UaNodeId &right) const
				{
					return
						std::string(left.toXmlString().toUtf8()) <
						std::string(right.toXmlString().toUtf8());
				}
			};

			/// Map for chaching super types. Key = Type, Value = Supertype
			std::map<UaNodeId, UaNodeId, UaNodeId_Compare> m_superTypes;

		private:
			static int PlatformLayerInitialized;

            void on_connected();

            UaClientSdk::SessionConnectInfo &
            prepareSessionConnectInfo(UaClientSdk::SessionConnectInfo &sessionConnectInfo) const;

            void initializeUpdateNamespaceCache(std::vector<std::string> &notFoundObjectTypeNamespaces);

            void findObjectTypeNamespaces(std::vector<std::string> &notFoundObjectTypeNamespaces, size_t i,
                                          const std::string &namespaceURI, std::shared_ptr<std::map <std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap);

            UaClientSdk::BrowseContext prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId);

            void browseTypes(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap, UaClientSdk::BrowseContext browseContext, UaNodeId startUaNodeId, const std::shared_ptr<ModelOpcUa::StructureBiNode>& parent);

            void handleContinuationPoint(const UaByteString &continuationPoint) const;

            void ReferenceDescriptionsToBrowseResults(const UaNodeId &typeDefinitionUaNodeId,
                                                      const UaReferenceDescriptions &referenceDescriptions,
                                                      std::list<ModelOpcUa::BrowseResult_t> &browseResult);

            std::list<ModelOpcUa::BrowseResult_t>
            BrowseWithContext(const ModelOpcUa::NodeId_t &startNode, const ModelOpcUa::NodeId_t &referenceTypeId,
                              const ModelOpcUa::NodeId_t &typeDefinition, UaClientSdk::BrowseContext &browseContext);

            UaClientSdk::BrowseContext prepareObjectTypeContext() const;

            std::shared_ptr<ModelOpcUa::StructureBiNode> handleBrowseTypeResult(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap,
                const ModelOpcUa::BrowseResult_t &entry, const std::shared_ptr<ModelOpcUa::StructureBiNode>& parent, ModelOpcUa::ModellingRule_t modellingRule);

            void createTypeMap(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap, std::shared_ptr<std::map<std::string, ModelOpcUa::StructureNode>> sharedPtr, uint16_t namespaceIndex);

            ModelOpcUa::ModellingRule_t browseModellingRule(UaNodeId uaNodeId);

            void split(const std::string &inputString, std::vector<std::string> &resultContainer, char delimiter);

            void updateResultContainer(const std::string &inputString, std::vector<std::string> &resultContainer,
                                       size_t current_char_position, size_t previous_char_position) const;

        };
	}
}
