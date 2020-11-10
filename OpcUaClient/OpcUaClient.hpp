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
		class OpcUaClient : public UaClientSdk::UaSessionCallback, public Dashboard::IDashboardDataClient {
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
			std::list<ModelOpcUa::BrowseResult_t>
			Browse(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId,
				   ModelOpcUa::NodeId_t typeDefinition) override;

			std::list<ModelOpcUa::BrowseResult_t>
			BrowseHasComponent(ModelOpcUa::NodeId_t startNode,
							   ModelOpcUa::NodeId_t typeDefinition) override;

			ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode,
															 ModelOpcUa::QualifiedName_t browseName) override;

			std::shared_ptr<ValueSubscriptionHandle>
			Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) override;

			uint GetImplementedNamespaceIndex(const ModelOpcUa::NodeId_t &nodeId) override;

			std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> nodeIds) override;

			std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &nodeId) override;

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) override;

			void CreateMachineListForNamespaceUnderStartNode(std::list<ModelOpcUa::BrowseResult_t> &machineList,
															 const std::string &startNodeNamespaceUri,
															 const ModelOpcUa::NodeId_t &startNode) override;

			void
			FillIdentificationValuesFromBrowseResult(std::list<ModelOpcUa::BrowseResult_t> &identification, std::list<ModelOpcUa::NodeId_t> &identificationNodes,
													 std::vector<std::string> &identificationValueKeys) override;
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
			std::vector<std::string> m_expectedObjectTypeNamespaces;
			OpcUa_MessageSecurityMode m_security = OpcUa_MessageSecurityMode::OpcUa_MessageSecurityMode_None;

			std::shared_ptr<std::thread> m_connectThread;
			std::shared_ptr<OpcUaInterface> m_opcUaWrapper;
			std::atomic_bool m_isConnected = {false};
			std::atomic_bool m_tryConnecting = {false};

			Subscription m_subscr;

			struct UaNodeId_Compare {
				bool operator()(const UaNodeId &left, const UaNodeId &right) const {
					return
							std::string(left.toXmlString().toUtf8()) <
							std::string(right.toXmlString().toUtf8());
				}
			};

			/// Map for chaching super types. Key = Type, Value = Supertype
			std::map<UaNodeId, UaNodeId, UaNodeId_Compare> m_superTypes;

		private:
			static int PlatformLayerInitialized;
			const ModelOpcUa::NodeId_t m_emptyId = ModelOpcUa::NodeId_t{"", ""};
			const ModelOpcUa::NodeId_t m_basicVariableTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/",
																					  "i=63"};
			const ModelOpcUa::NodeId_t m_basicObjectTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/",
																					"i=58"};

			void on_connected();
			UaDataValues readValues2(std::list<ModelOpcUa::NodeId_t> modelNodeIds);


			static UaClientSdk::SessionConnectInfo &
			prepareSessionConnectInfo(UaClientSdk::SessionConnectInfo &sessionConnectInfo);

			void initializeNamespaceCache(std::vector<std::string> &notFoundObjectTypeNamespaces);

			void
			findObjectTypeNamespacesAndCreateTypeMap(std::vector<std::string> &notFoundObjectTypeNamespaces, size_t i,
													 const std::string &namespaceURI,
													 std::shared_ptr<std::map<std::string,
															 std::shared_ptr<ModelOpcUa::StructureBiNode>>

													 >
													 bidirectionalTypeMap =
													 std::make_shared<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>
													 >());

			UaClientSdk::BrowseContext prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId);

			void browseTypes(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>

			> bidirectionalTypeMap,
							 const UaClientSdk::BrowseContext &browseContext,
							 const UaNodeId &startUaNodeId,
							 const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent,
							 bool ofBaseDataVariableType
			);

			static void handleContinuationPoint(const UaByteString &continuationPoint);

			void ReferenceDescriptionsToBrowseResults(const UaNodeId &typeDefinitionUaNodeId,
													  const UaReferenceDescriptions &referenceDescriptions,
													  std::list<ModelOpcUa::BrowseResult_t> &browseResult);

			std::list<ModelOpcUa::BrowseResult_t>
			BrowseWithContext(const ModelOpcUa::NodeId_t &startNode, const ModelOpcUa::NodeId_t &referenceTypeId,
							  const ModelOpcUa::NodeId_t &typeDefinition, UaClientSdk::BrowseContext &browseContext);

			static UaClientSdk::BrowseContext prepareObjectAndVariableTypeBrowseContext();

			std::shared_ptr<ModelOpcUa::StructureBiNode> handleBrowseTypeResult(
					std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>

					> &bidirectionalTypeMap,
					const ModelOpcUa::BrowseResult_t &entry,
					const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent, ModelOpcUa::ModellingRule_t
					modellingRule,
					bool ofBaseDataVariableType
			);

			void browseUnderStartNode(UaNodeId startUaNodeId, UaReferenceDescriptions &referenceDescriptions);

			void browseUnderStartNode(UaNodeId startUaNodeId, UaReferenceDescriptions &referenceDescriptions,
									  UaClientSdk::BrowseContext browseContext);

			ModelOpcUa::BrowseResult_t
			ReferenceDescriptionToBrowseResult(const OpcUa_ReferenceDescription &referenceDescriptions);

			static void
			createTypeMap(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>

			> &bidirectionalTypeMap, const std::shared_ptr<std::map<std::string,
					std::shared_ptr<ModelOpcUa::StructureNode>>> &sharedPtr,
						  uint16_t namespaceIndex
			);

			ModelOpcUa::ModellingRule_t browseModellingRule(const UaNodeId &uaNodeId);

			static void
			split(const std::string &inputString, std::vector<std::string> &resultContainer, char delimiter);

			static void
			updateResultContainer(const std::string &inputString, std::vector<std::string> &resultContainer,
								  size_t current_char_position, size_t previous_char_position);

			void fillNamespaceCache(const UaStringArray &uaNamespaces);

			void browseObjectOrVariableTypeAndFillBidirectionalTypeMap(const ModelOpcUa::NodeId_t &basicTypeNode,
																	   std::shared_ptr<std::map<std::string,
																			   std::shared_ptr<
																					   ModelOpcUa::StructureBiNode>>

																	   > bidirectionalTypeMap,
																	   bool ofBaseDataVariableType
			);

			void updateTypeMap();
		};
	}
}
