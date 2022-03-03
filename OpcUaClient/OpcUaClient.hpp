 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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
#include "TypeDictionary/TypeDictionary.hpp"
#include "Subscription.hpp"
#include "OpcUaInterface.hpp"
#include <functional>

namespace Umati
{

	namespace OpcUa
	{
		class OpcUaClient : public Dashboard::IDashboardDataClient
		{

		public:
			explicit OpcUaClient(std::string serverURI, std::function<void()> issueReset, std::string Username = std::string(),
								 std::string Password = std::string(), std::uint8_t security = 1,
								 std::vector<std::string> expectedObjectTypeNamespaces = std::vector<std::string>(),
								 std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>(),
								 bool bypassCertVerification = false);
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
			Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) override;

			void Unsubscribe(std::vector<int32_t>monItemIds, std::vector<int32_t> clientHandle) override;

			std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds) override;

			std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &_nodeId) override;

			std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) override;

			std::vector<std::string> Namespaces() override;

			bool VerifyConnection() override;

            bool isSameOrSubtype(const ModelOpcUa::NodeId_t &expectedType, const ModelOpcUa::NodeId_t &checkType,
                                 size_t maxDepth) override;

			void buildCustomDataTypes() override;

			void readTypeDictionaries() override;

			void updateCustomTypes() override;

		protected:
			void connectionStatusChanged(UA_Int32 clientConnectionId, UA_ServerState serverStatus);

			bool connect();

			UA_NodeClass readNodeClass(const open62541Cpp::UA_NodeId &nodeId);

			void checkConnection();

			open62541Cpp::UA_NodeId browseSuperType(const open62541Cpp::UA_NodeId &typeNodeId);

			// Max search depth
			bool isSameOrSubtype(const open62541Cpp::UA_NodeId &expectedType, const open62541Cpp::UA_NodeId &checkType, std::size_t maxDepth = 100);

			double m_maxAgeRead_ms = 100.0;

			void updateNamespaceCache();
			/// Ensure that the new namespace chache is compatible to the current class state.
			/// Verifies, that no namespace has been removed, or reordered.
			bool verifyCompatibleNamespaceCache(std::map<uint16_t, std::string> oldIndexToUriCache);

			void threadConnectExecution();

			std::function<void()> m_issueReset;
			std::map<uint16_t, std::string> m_indexToUriCache;
			std::string m_serverUri;
			std::string m_username;
			std::string m_password;
			UA_MessageSecurityMode m_security = UA_MESSAGESECURITYMODE_NONE;
			UA_DataTypeArray *m_dataTypeArray;
			std::vector<TypeDictionary::TypeDictionary> m_ptdv;

			std::shared_ptr<std::thread> m_connectThread;
			std::shared_ptr<OpcUaInterface> m_opcUaWrapper;
			std::atomic_bool m_isConnected = {false};
			std::atomic_bool m_tryConnecting = {false};

			Subscription m_subscr;

			const std::map<std::string, size_t> XMLtoUaType={
				{ "opc:Boolean", UA_TYPES_BOOLEAN },
				{ "opc:SByte", UA_TYPES_SBYTE },
				{ "opc:Byte", UA_TYPES_BYTE },
				{ "opc:Int16", UA_TYPES_INT16 },
				{ "opc:UInt16", UA_TYPES_UINT16 },
				{ "opc:Int32", UA_TYPES_INT32 },
				{ "opc:UInt32", UA_TYPES_UINT32 },
				{ "opc:Int64", UA_TYPES_INT64 },
				{ "opc:UInt64", UA_TYPES_UINT64 },
				{ "opc:Float", UA_TYPES_FLOAT },
				{ "opc:Double", UA_TYPES_DOUBLE },
				{ "opc:CharArray", UA_TYPES_STRING },
				{ "opc:DateTime", UA_TYPES_DATETIME },
				{ "opc:Guid", UA_TYPES_GUID },
				{ "opc:ByteString", UA_TYPES_BYTESTRING },
				{ "ua:XmlElement", UA_TYPES_XMLELEMENT },
				{ "ua:NodeId", UA_TYPES_NODEID },
				{ "ua:ExpandedNodeId", UA_TYPES_EXPANDEDNODEID },
				{ "ua:StatusCode", UA_TYPES_STATUSCODE },
				{ "ua:QualifiedName", UA_TYPES_QUALIFIEDNAME },
				{ "ua:LocalizedText", UA_TYPES_LOCALIZEDTEXT },
				{ "ua:ExtensionObject", UA_TYPES_EXTENSIONOBJECT },
				{ "ua:DataValue", UA_TYPES_DATAVALUE },
				{ "ua:Variant", UA_TYPES_VARIANT },
				{ "ua:DiagnosticInfo", UA_TYPES_DIAGNOSTICINFO },

				/* TODO ??? 
				{ "", UA_TYPES_DECIMAL },
				{ "", UA_TYPES_ENUM },
				{ "", UA_TYPES_STRUCTURE },
				{ "", UA_TYPES_OPTSTRUCT }, 
				{ "", UA_TYPES_UNION },
				{ "", UA_TYPES_BITFIELDCLUSTER } */
			};

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
			std::shared_ptr<UA_Client> m_pClient; // Zugriff aus dem ConnectThread, dem PublisherThread
            std::recursive_mutex m_clientMutex;
		private:
			void on_connected();

			std::vector<nlohmann::json> readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds);

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
			void updateCustomDataTypesNamespace(const std::string namespaceURI, const std::size_t namespaceIndex);
        };
	} // namespace OpcUa
} // namespace Umati
