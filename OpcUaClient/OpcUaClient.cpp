 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2022 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include <tinyxml2.h>
#include "OpcUaClient.hpp"
#include "ScopeExitGuard.hpp"
#include "SetupSecurity.hpp"

#include <Exceptions/ClientNotConnected.hpp>
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Converter/ModelQualifiedNameToUaQualifiedName.hpp"
#include "Converter/UaQualifiedNameToModelQualifiedName.hpp"
#include "Converter/UaNodeClassToModelNodeClass.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"
#include "Converter/CustomDataTypes/types_machinery_result_generated_handling.h"
#include "Converter/CustomDataTypes/types_tightening_generated_handling.h"


namespace Umati
{

	namespace OpcUa
	{


		static void stateCallback(UA_Client *client,
                          UA_SecureChannelState channelState,
                          UA_SessionState sessionState,
                          UA_StatusCode connectState)
		{
			switch(channelState) {
				case UA_SECURECHANNELSTATE_FRESH:
				case UA_SECURECHANNELSTATE_CLOSED:
					LOG(INFO) << "The client is disconnected";
					break;
				case UA_SECURECHANNELSTATE_HEL_SENT:
					LOG(INFO) << "Waiting for ack";
					break;
				case UA_SECURECHANNELSTATE_OPN_SENT:
					LOG(INFO) << "Waiting for OPN Response";
					break;
				case UA_SECURECHANNELSTATE_OPEN:
					LOG(INFO) << "A SecureChannel to the server is open";
					break;
				default:
					break;
				}

			switch(sessionState) {
    			case UA_SESSIONSTATE_ACTIVATED:
      				LOG(INFO) << "A session with the server is activated";
					break;
				case UA_SESSIONSTATE_CLOSED:
					LOG(INFO) << "Session disconnected";
					break;
				default:
					break;
				}

			switch(connectState){
				case UA_STATUSCODE_BADDISCONNECT:
				LOG(INFO) << "Bad Disconnect";
			}
		}

		static void inactivityCallback (UA_Client *client) {
		    LOG(ERROR) << "\n\n\nINACTIVITYCALLBACK\n\n\n";
		}

		UA_DataTypeArray TighteningSystemTypes = {NULL, 1, UA_TYPES_TIGHTENING};
		static UA_DataTypeArray getMachineryResultTypes () {
			return {&TighteningSystemTypes, 5, UA_TYPES_MACHINERY_RESULT};
		}

		OpcUaClient::OpcUaClient(std::string serverURI, std::function<void()> issueReset,
								 std::string Username, std::string Password,
								 std::uint8_t security, std::vector<std::string> expectedObjectTypeNamespaces,
								 std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper, bool bypassCertVerification)
			: m_issueReset(issueReset),
			m_serverUri(std::move(serverURI)), m_username(std::move(Username)), m_password(std::move(Password)),
			m_security(static_cast<UA_MessageSecurityMode>(security)),
			m_subscr(m_uriToIndexCache, m_indexToUriCache),
			m_pClient(UA_Client_new(), UA_Client_delete)
        {
			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);
				UA_ClientConfig *config = UA_Client_getConfig(m_pClient.get());
				if (bypassCertVerification) {
					config->certificateVerification.verifyCertificate = &bypassVerify;
				}
				SetupSecurity::setupSecurity(config, m_pClient.get());
				config->securityMode = UA_MessageSecurityMode(security);
				UA_ApplicationDescription_clear(&config->clientDescription);
				prepareSessionConnectInfo(config->clientDescription);
				config->timeout = 2000;
				config->inactivityCallback = inactivityCallback;
				config->stateCallback = stateCallback;
				config->customDataTypes = &m_dataTypeArray;
			}

			m_opcUaWrapper = std::move(opcUaWrapper);
			m_opcUaWrapper->setSubscription(&m_subscr);
			m_tryConnecting = true;
			// Try connecting at least once
			this->connect();
			m_connectThread = std::make_shared<std::thread>([this]() { this->threadConnectExecution(); });
		}

		void OpcUaClient::updateCustomTypes()
		{
			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);
				UA_ClientConfig *config = UA_Client_getConfig(m_pClient.get());
				config->customDataTypes = m_dataTypeArray;
			}
		}

		bool OpcUaClient::connect()
		{
			open62541Cpp::UA_String sURL(m_serverUri.c_str());
			UA_StatusCode  result;

			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);
				if(m_username.empty() && m_password.empty()){
					result = m_opcUaWrapper->SessionConnect(m_pClient.get(), sURL);
				}else{
					result = m_opcUaWrapper->SessionConnectUsername(m_pClient.get(), sURL, m_username, m_password);
				}
			}
			if (UA_StatusCode_isBad(result))
			{
				LOG(ERROR) << "Connecting failed in OPC UA Data Client: " << UA_StatusCode_name(result) << std::endl;
				connectionStatusChanged(0,UA_SERVERSTATE_FAILED);
				//VERIFY move this to the connectionStatsuChanged callback?
				if(result == UA_STATUSCODE_BADDISCONNECT || result == UA_STATUSCODE_BADUSERACCESSDENIED || result == UA_STATUSCODE_BADCONNECTIONCLOSED || result == UA_STATUSCODE_BADAPPLICATIONSIGNATUREINVALID){
					m_tryConnecting = false;
				}
				return false;
			}
			connectionStatusChanged(0,UA_SERVERSTATE_RUNNING);

			return true;
		}
		UA_ApplicationDescription &
		OpcUaClient::prepareSessionConnectInfo(UA_ApplicationDescription &sessionConnectInfo)
		{
			sessionConnectInfo.applicationName = UA_LOCALIZEDTEXT_ALLOC("en-US", "KonI4.0 OPC UA Data Client");
			sessionConnectInfo.applicationUri = UA_STRING_ALLOC("http://dashboard.umati.app/OPCUA_DataClient");
		 	sessionConnectInfo.productUri = UA_STRING_ALLOC("KonI40OpcUaClient_Product");
			sessionConnectInfo.applicationType = UA_APPLICATIONTYPE_CLIENT;
			return sessionConnectInfo;
		}

		void OpcUaClient::on_connected()
		{
			updateNamespaceCache();
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			m_opcUaWrapper->SubscriptionCreateSubscription(m_pClient.get());
		}

		std::string OpcUaClient::getTypeName(const ModelOpcUa::NodeId_t &nodeId)
		{
			return readNodeBrowseName(nodeId);
		}

		std::string OpcUaClient::readNodeBrowseName(const ModelOpcUa::NodeId_t &_nodeId)
		{

			auto nodeId = Converter::ModelNodeIdToUaNodeId(_nodeId, m_uriToIndexCache).getNodeId();
			checkConnection();

			UA_QualifiedName resultname;
			UA_QualifiedName_init(&resultname);
			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);
				auto uaResult = UA_Client_readBrowseNameAttribute(m_pClient.get(), *nodeId.NodeId, &resultname);

				if (UA_StatusCode_isBad(uaResult))
				{
					LOG(ERROR) << "readNodeClass failed for node: '" << nodeId.NodeId->identifier.string.data
							<< "' with " << UA_StatusCode_name(uaResult);
					UA_QualifiedName_clear(&resultname);
					throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
				}
			}
			std::string resName = std::string((char *)resultname.name.data,resultname.name.length);

			UA_QualifiedName_clear(&resultname);

			return _nodeId.Uri + ";" + resName;
		}

		UA_NodeClass OpcUaClient::readNodeClass(const open62541Cpp::UA_NodeId &nodeId)
		{
			checkConnection();

			UA_NodeClass returnClass;
			UA_NodeClass_init(&returnClass);
			try{
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			auto uaResult = UA_Client_readNodeClassAttribute(m_pClient.get(), *nodeId.NodeId, &returnClass);
			if (UA_StatusCode_isBad(uaResult))
			{
				LOG(ERROR) << "readNodeClass failed";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}
			}catch(...){
				LOG(ERROR) << "readNodeClass failed";
			}
			return returnClass;
		}

		void OpcUaClient::checkConnection()
		{
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			if (!this->m_isConnected || !m_opcUaWrapper->SessionIsConnected(m_pClient.get()))
			{
				connectionStatusChanged(0,UA_SERVERSTATE_FAILED);
				throw Exceptions::ClientNotConnected("Need connected client.");
			}
		}

		open62541Cpp::UA_NodeId OpcUaClient::browseSuperType(const open62541Cpp::UA_NodeId &typeNodeId)
		{
			open62541Cpp::UA_NodeId referenceTypeUaNodeId;
			referenceTypeUaNodeId.NodeId->identifierType = UA_NODEIDTYPE_NUMERIC;
			referenceTypeUaNodeId.NodeId->identifier.numeric = UA_NS0ID_HASSUBTYPE;

			UA_BrowseDescription browseContext;
			browseContext.browseDirection = UA_BROWSEDIRECTION_INVERSE;
			browseContext.includeSubtypes = UA_TRUE;

			browseContext.nodeClassMask = 0; // ALL
			UA_NodeId_copy(referenceTypeUaNodeId.NodeId,&browseContext.referenceTypeId);
			browseContext.resultMask = UA_BROWSERESULTMASK_NONE;

			UA_NodeClass nodeClass = readNodeClass(typeNodeId);

			switch (nodeClass)
			{
			case UA_NODECLASS_OBJECTTYPE:
			case UA_NODECLASS_VARIABLETYPE:
			{
				browseContext.nodeClassMask = nodeClass;
				break;
			}
			default:
				LOG(ERROR) << "Invalid NodeClass " << nodeClass;
				throw Exceptions::UmatiException("Invalid NodeClass");
			}


			UA_ByteString continuationPoint;
			std::vector<UA_ReferenceDescription> referenceDescriptions;
			UA_BrowseResponse uaResult;
			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);

				uaResult = m_opcUaWrapper->SessionBrowse(m_pClient.get(), /*m_defaultServiceSettings,*/ typeNodeId, browseContext,
														  continuationPoint, referenceDescriptions);
			}
			if (UA_StatusCode_isBad(uaResult.results->statusCode))
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.results->statusCode;
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult.results->statusCode);
			}

			UA_BrowseResponse_clear(&uaResult);
			std::list<ModelOpcUa::BrowseResult_t> browseResult;
			if (referenceDescriptions.size() == 0)
			{
				return open62541Cpp::UA_NodeId();
			}
			if (referenceDescriptions.size() > 1)
			{
				LOG(ERROR) << "Found multiple superTypes for " << typeNodeId.NodeId->identifier.string.data;
				return open62541Cpp::UA_NodeId();
			}

			UA_ExpandedNodeId retExpandedNodeId;
			UA_ExpandedNodeId_copy(&referenceDescriptions.at(0).nodeId,&retExpandedNodeId);

			return open62541Cpp::UA_NodeId(retExpandedNodeId.nodeId);
		}

		bool
		OpcUaClient::isSameOrSubtype(const open62541Cpp::UA_NodeId &expectedType, const open62541Cpp::UA_NodeId &checkType, std::size_t maxDepth)
		{

			UA_NodeId checkNode = *checkType.NodeId;
			UA_NodeId expectedNode = *expectedType.NodeId;

			if (UA_NodeId_isNull(&checkNode))
			{
				return false;
			}

			if (UA_NodeId_equal(&expectedNode, &checkNode))
			{
				return true;
			}

			auto it = m_superTypes.find(checkType);

			if (it != m_superTypes.end())
			{
				return isSameOrSubtype(expectedType, it->second, --maxDepth);
			}

			auto superType = browseSuperType(checkType);
			m_superTypes[checkType] = superType;
			return isSameOrSubtype(expectedType, superType, --maxDepth);
		}

        bool OpcUaClient::isSameOrSubtype(
                const ModelOpcUa::NodeId_t &expectedType,
                const ModelOpcUa::NodeId_t &checkType,
                std::size_t maxDepth) {
            auto expectedTypeUa = Converter::ModelNodeIdToUaNodeId(expectedType, m_uriToIndexCache).getNodeId();
            auto checkTypeUa = Converter::ModelNodeIdToUaNodeId(checkType, m_uriToIndexCache).getNodeId();
            bool ret;
            try {
                ret = isSameOrSubtype(expectedTypeUa, checkTypeUa, maxDepth);
            } catch(std::exception &e) {
                ret = false;
            }
            return ret;
        }

		void OpcUaClient::initializeNamespaceCache()
		{
            std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			m_opcUaWrapper->SessionUpdateNamespaceTable(m_pClient.get());

			m_uriToIndexCache.clear();
			m_indexToUriCache.clear();
		}

		void OpcUaClient::updateNamespaceCache()
		{
			auto existingIndexToUri = m_indexToUriCache;
			initializeNamespaceCache();

			auto uaNamespaces = m_opcUaWrapper->SessionGetNamespaceTable();

			fillNamespaceCache(uaNamespaces);
			if(!verifyCompatibleNamespaceCache(existingIndexToUri)) {
				m_issueReset();
			}
		}

		bool OpcUaClient::verifyCompatibleNamespaceCache(std::map<uint16_t, std::string> oldIndexToUriCache) {
			for(const auto &pair : oldIndexToUriCache){
				auto newEntry = m_indexToUriCache.find(pair.first);
				if(newEntry == m_indexToUriCache.end()) {
					LOG(INFO) << "Incompatible Namespace change detected. "
								<<"Namespaces index no longer available: " << pair.first;
					return false;
				}
				if(newEntry->second != pair.second) {
					LOG(INFO) << "Incompatible Namespace change detected. "
								<<"Namespaces uri chaned: " << pair.second << "!= " << newEntry->second;
					return false;
				}
			}
			return true;
		}

		void OpcUaClient::fillNamespaceCache(const std::vector<std::string> &uaNamespaces)
		{
			for(size_t i = 0; i < uaNamespaces.size(); ++i){

				std::string namespaceURI = uaNamespaces.at(i);

				if ( m_uriToIndexCache.find(namespaceURI) == m_uriToIndexCache.end() ) {
				    m_uriToIndexCache[namespaceURI] = static_cast<uint16_t>(i);
					m_indexToUriCache[static_cast<uint16_t>(i)] = namespaceURI;

					updateCustomDataTypesNamespace(namespaceURI, i);
				} else {
                    LOG(INFO) << "Namespace already in cache";
				}

				LOG(INFO) << "index: " << std::to_string(i) << ", namespaceURI: " << namespaceURI;
			}

		}

		void OpcUaClient::updateCustomDataTypesNamespace(std::string namespaceURI, std::size_t namespaceIndex)
		{
			if (namespaceURI == "http://opcfoundation.org/UA/Machinery/Result/") {
				uint16_t nsIdx = static_cast<uint16_t>(namespaceIndex);

				for(size_t j = 0; j < UA_TYPES_MACHINERY_RESULT_COUNT; j++) {
					UA_TYPES_MACHINERY_RESULT[j].typeId.namespaceIndex = nsIdx;
					UA_TYPES_MACHINERY_RESULT[j].binaryEncodingId.namespaceIndex = nsIdx;
				}

				m_dataTypeArray.types = UA_TYPES_MACHINERY_RESULT;
			}

			if (namespaceURI == "http://opcfoundation.org/UA/IJT/") {
				uint16_t nsIdx = static_cast<uint16_t>(namespaceIndex);

				for(size_t j = 0; j < UA_TYPES_TIGHTENING_COUNT; j++) {
					UA_TYPES_TIGHTENING[j].typeId.namespaceIndex = nsIdx;
					UA_TYPES_TIGHTENING[j].binaryEncodingId.namespaceIndex = nsIdx;
				}
			}
		}

		ModelOpcUa::ModellingRule_t OpcUaClient::browseModellingRule(const open62541Cpp::UA_NodeId &uaNodeId)
		{
			UA_ByteString continuationPoint;
			std::vector<UA_ReferenceDescription> referenceDescriptions;

			/// begin browse modelling rule
			UA_BrowseDescription browseContext2 = getUaBrowseContext(prepareObjectAndVariableTypeBrowseContext());
			browseContext2.referenceTypeId.identifier.numeric= UA_NS0ID_HASMODELLINGRULE;
			ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::Optional;
			UA_BrowseResponse uaResult2;
			{
            std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			uaResult2 = m_opcUaWrapper->SessionBrowse(m_pClient.get(), /*m_defaultServiceSettings,*/ uaNodeId,
														   browseContext2,
														   continuationPoint, referenceDescriptions);
			}
			if (UA_StatusCode_isBad(uaResult2.results->statusCode))
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult2.results->statusCode << "for nodeId"
						   << uaNodeId.NodeId->identifier.string.data;
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult2.results->statusCode);
			}

			UA_BrowseDescription_clear(&browseContext2);
			UA_BrowseResponse_clear(&uaResult2);

			for (UA_Int32 i = 0; i < referenceDescriptions.size(); i++)
			{
				auto refDescr = referenceDescriptions.at(i);
				ModelOpcUa::BrowseResult_t browseResult = ReferenceDescriptionToBrowseResult(refDescr);
				if (browseResult.BrowseName.Name == "Mandatory")
				{
					modellingRule = ModelOpcUa::Mandatory;
				}
				else if (browseResult.BrowseName.Name == "Optional")
				{
					modellingRule = ModelOpcUa::Optional;
				}
				else if (browseResult.BrowseName.Name == "MandatoryPlaceholder")
				{
					modellingRule = ModelOpcUa::MandatoryPlaceholder;
				}
				else if (browseResult.BrowseName.Name == "OptionalPlaceholder")
				{
					modellingRule = ModelOpcUa::OptionalPlaceholder;
				}
			}
			return modellingRule;
		}

		Umati::Dashboard::IDashboardDataClient::BrowseContext_t OpcUaClient::prepareObjectAndVariableTypeBrowseContext()
		{
			Umati::Dashboard::IDashboardDataClient::BrowseContext_t ret;
			ret.nodeClassMask =
				(std::uint32_t)Umati::Dashboard::IDashboardDataClient::BrowseContext_t::NodeClassMask::OBJECT |
				(std::uint32_t)Umati::Dashboard::IDashboardDataClient::BrowseContext_t::NodeClassMask::OBJECT_TYPE |
				(std::uint32_t)Umati::Dashboard::IDashboardDataClient::BrowseContext_t::NodeClassMask::VARIABLE |
				(std::uint32_t)Umati::Dashboard::IDashboardDataClient::BrowseContext_t::NodeClassMask::VARIABLE_TYPE;
			return ret;
		}

		void OpcUaClient::connectionStatusChanged(UA_Int32  /*clientConnectionId*/,UA_ServerState serverStatus)
		{
			switch (serverStatus)
			{
			case UA_SERVERSTATE_FAILED:
				LOG(ERROR) << "Disconnected." << std::endl;
				m_isConnected = false;
				break;
			case UA_SERVERSTATE_RUNNING:
				LOG(ERROR) << "Connected." << std::endl;
				m_isConnected = true;
				on_connected();
				break;
			case UA_SERVERSTATE_SHUTDOWN:
				LOG(ERROR) << "ServerShutdown." << std::endl;
				break;
			}
		}

		void OpcUaClient::buildCustomDataTypes()
		{	
			/* TODO: Handle Cross Referencing TypeDictionaries! */
			m_dataTypeArray = (UA_DataTypeArray *)malloc(sizeof(UA_DataTypeArray) * 10);
			UA_DataTypeArray *lastElement = nullptr;

			for (auto &td : m_ptdv) {
				for (auto &st : td.StructuredTypes)	{
					st.Kind = UA_DATATYPEKIND_STRUCTURE;
					for (auto &member : st.Fields) {
						if (member.TypeName == "opc:Bit") {
							member.redundant = true;
							st.Kind = UA_DATATYPEKIND_OPTSTRUCT;
						}
						if (member.LengthField != "") {
							for (auto &member1 : st.Fields) {
								if (member.LengthField == member1.Name)	{
									member1.redundant = true;
								}
							}
						}
					}
					st.Fields.erase(std::remove_if(st.Fields.begin(), st.Fields.end(), [](const auto &x){ return x.redundant; }), st.Fields.end());
				}
			}

			for (auto &td : m_ptdv)	{
				const size_t noEnums = td.EnumeratedTypes.size();
				const size_t noStructs = td.StructuredTypes.size();
				const size_t size = noStructs + td.EnumeratedTypes.size();
				UA_DataType *types = new UA_DataType[size];
				const UA_DataTypeArray *next = lastElement;
				std::vector<std::tuple<UA_DataType *, UA_DataTypeMember *, TypeDictionary::Field *>> withUnsatisfiedMemberType;
				std::map<std::string, UA_DataType *> dataTypeNameToDataType;

				for (size_t i = 0; i < noEnums; i++) {
					auto &st = td.EnumeratedTypes[i];
					types[i].typeName = st.Name.c_str();
					types[i].typeId = *Converter::ModelNodeIdToUaNodeId(st.NodeId, m_uriToIndexCache).getNodeId().NodeId;
					UA_NodeId bNodeId = types[i].typeId;
					bNodeId.identifier.numeric = 0;
					types[i].binaryEncodingId = bNodeId;
					types[i].typeKind = UA_DATATYPEKIND_ENUM;
					types[i].overlayable = UA_BINARY_OVERLAYABLE_INTEGER;
					types[i].membersSize = 0;
					types[i].members = nullptr;
					types[i].memSize = sizeof(UA_Int32);
					types[i].pointerFree = true;
					dataTypeNameToDataType.insert({std::string("tns:") + types[i].typeName, &types[i]});
				}

				for (size_t i = 0; i < noStructs; i++) {
					auto &st = td.StructuredTypes[i];
					types[i + noEnums].typeName = st.Name.c_str();
					types[i + noEnums].membersSize = st.Fields.size();
					types[i + noEnums].binaryEncodingId = *Converter::ModelNodeIdToUaNodeId(st.BinaryNodeId, m_uriToIndexCache).getNodeId().NodeId;
					types[i + noEnums].typeId = *Converter::ModelNodeIdToUaNodeId(st.NodeId, m_uriToIndexCache).getNodeId().NodeId;
					types[i + noEnums].typeKind = st.Kind;
					types[i + noEnums].pointerFree = false;
					types[i + noEnums].overlayable = false;
					types[i + noEnums].members = new UA_DataTypeMember[st.Fields.size()];
					dataTypeNameToDataType.insert({std::string("tns:") + types[i + noEnums].typeName, &types[i + noEnums]});

					for (size_t j = 0; j < st.Fields.size(); j++) {
						types[i + noEnums].members[j].isArray = !st.Fields[j].LengthField.empty();
						types[i + noEnums].members[j].isOptional = !st.Fields[j].SwitchField.empty();
						auto it = XMLtoUaType.find(st.Fields[j].TypeName);
						if (it == XMLtoUaType.end()) {
							withUnsatisfiedMemberType.push_back(std::make_tuple(&types[i + noEnums], &types[i + noEnums].members[j], &st.Fields[j]));
						}
						else {
							types[i + noEnums].members[j].memberType = &UA_TYPES[it->second];
						}
						types[i + noEnums].members[j].memberName = st.Fields[j].Name.c_str();
					}
				}

				TypeDictionary::DependecyGraph<UA_DataType> depGraph{};
				for (auto &uns : withUnsatisfiedMemberType)	{
					auto targetDataType = dataTypeNameToDataType.at(std::get<2>(uns)->TypeName);
					std::get<1>(uns)->memberType = targetDataType;
					depGraph.addEdge(targetDataType, std::get<0>(uns));
				}

				depGraph.topologicalSort();
				auto &result = depGraph.getResult();

				while (!result.empty())	{
					auto res = result.top();
					UA_UInt32 padding = 0;
					for (size_t i = 0; i < res->membersSize; i++) {	
						res->members[i].padding = padding;
						if (res->members[i].isArray) {
							padding += sizeof(void *);
							padding += sizeof(size_t);
						}
						else if (res->members[i].isOptional) {
							padding += sizeof(void *);
						}
						else {
							padding += res->members[i].memberType->memSize;
						}
					}
					res->memSize = padding;
					result.pop();
				}

				for (size_t i = 0; i < noStructs; i++) {
					auto &st = td.StructuredTypes[i];
					UA_Byte padding = 0;
					for (size_t j = 0; j < st.Fields.size(); j++) {	
						if(j > 0) types[i + noEnums].members[j - 1].padding = 0;
						types[i + noEnums].members[j].padding = padding;
						if (types[i + noEnums].members[j].isArray) {
							padding += sizeof(size_t);
							padding += sizeof(void *);
						}
						else if (types[i + noEnums].members[j].isOptional) {
							padding += sizeof(void *);
						}
						else {
							padding += types[i + noEnums].members[j].memberType->memSize;
						}
					}
					types[i + noEnums].memSize = padding;
					types[i + noEnums].members[st.Fields.size() - 1].padding = 0;
				}
				m_dataTypeArray = new UA_DataTypeArray{lastElement, size, types};
				lastElement = m_dataTypeArray;
			}
		}

		void OpcUaClient::readTypeDictionaries()
		{

			std::map<ModelOpcUa::QualifiedName_t, ModelOpcUa::NodeId_t> nameToNodeId{};
			std::map<ModelOpcUa::QualifiedName_t, ModelOpcUa::NodeId_t> nameToBinaryNodeId{};
			const char *buffer = new char[1000];
			UA_NodeId nodeId;
			UA_Variant v;

			UA_NodeId_init(&nodeId);
			UA_Variant_init(&v);

			typedef struct {
				size_t visitedElements;
				tinyxml2::XMLElement *current;
			} XMLIterator;

			auto dictionaryResults = Browse(Dashboard::NodeId_OPC_Binary, Dashboard::IDashboardDataClient::BrowseContext_t::HasComponent());
			for (auto &dict : dictionaryResults)
			{
				if (dict.BrowseName.Name == "Opc.Ua" || dict.BrowseName.Name == "Opc.Ua.Di") continue;

				TypeDictionary::TypeDictionary td{};
				auto nodesInTypeDict = Browse(dict.NodeId, Dashboard::IDashboardDataClient::BrowseContext_t::Hierarchical());
				
				for (auto &node : nodesInTypeDict)
				{
					if (node.BrowseName == ModelOpcUa::QualifiedName_t{"", "NamespaceUri"})	{}
					else if (node.TypeDefinition.Id == Dashboard::NodeId_DataTypeDescriptionType.Id)
					{
						auto describedNode = Browse(node.NodeId, Dashboard::IDashboardDataClient::BrowseContext_t::DescriptionOf()).front();
						auto encodedNode = Browse(describedNode.NodeId, Dashboard::IDashboardDataClient::BrowseContext_t::EncodingOf()).front();
						auto nameOfDescribedStructuredType = node.BrowseName;
						nameToNodeId.insert({nameOfDescribedStructuredType, encodedNode.NodeId});
						nameToBinaryNodeId.insert({nameOfDescribedStructuredType, describedNode.NodeId});
					}
				}
				/* Search for EnumTypes */
				tinyxml2::XMLError eResult;
				tinyxml2::XMLDocument xml_file;
				tinyxml2::XMLElement *xml_element;
				nodeId = *Converter::ModelNodeIdToUaNodeId(dict.NodeId, m_uriToIndexCache).getNodeId().NodeId;
   				UA_Client_readValueAttribute(m_pClient.get(), nodeId, &v);
				UA_String s = *(UA_String *)v.data;
				std::string typeDictXMLString((char *)s.data, s.length);			

				eResult = xml_file.Parse(typeDictXMLString.c_str());
				if (eResult != tinyxml2::XML_SUCCESS)
				{
					LOG(ERROR) << "Error parsing TypeDictionary XML " << dict.NodeId;
					continue;
				}

				xml_element = xml_file.RootElement();
				if (xml_element == nullptr)
				{	
					LOG(ERROR) << "Error finding XMLRootElement of TypeDictionary " << dict.NodeId;
					continue;
				}

				eResult = xml_element->QueryStringAttribute("TargetNamespace", &buffer);
				if (eResult != tinyxml2::XML_SUCCESS) {
					LOG(ERROR) << "Error reading TargetNamespace in XML of TypeDictionary " << dict.NodeId;
					continue;
				}
				td.TargetNamespace = std::string(buffer);

				auto firstNode = xml_element->FirstChildElement("opc:StructuredType");
				for (XMLIterator it = {.visitedElements = 0, .current = firstNode};
					 it.current != nullptr || (it.current != firstNode && it.visitedElements != 0);
					 it = {.visitedElements = it.visitedElements++, .current = it.current->NextSiblingElement("opc:StructuredType")})
				{
					TypeDictionary::StructuredType stype;
					eResult = it.current->QueryStringAttribute("BaseType", &buffer);
					if (eResult != tinyxml2::XML_SUCCESS)
						continue;
					stype.BaseType = std::string(buffer);
					eResult = it.current->QueryStringAttribute("Name", &buffer);
					if (eResult != tinyxml2::XML_SUCCESS)
						continue;
					stype.Name = std::string(buffer);
					auto firstNodeInner = it.current->FirstChildElement("opc:Field");
					for (XMLIterator it2 = {.visitedElements = 0, .current = firstNodeInner};
						 it2.current != nullptr || (it2.current != firstNodeInner && it2.visitedElements != 0);
						 it2 = {.visitedElements = it2.visitedElements++, .current = it2.current->NextSiblingElement("opc:Field")})
					{
						TypeDictionary::Field field{};
						eResult = it2.current->QueryStringAttribute("TypeName", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS)
							field.TypeName = std::string(buffer);
						eResult = it2.current->QueryStringAttribute("Name", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS)
							field.Name = std::string(buffer);
						eResult = it2.current->QueryStringAttribute("LengthField", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS)
							field.LengthField = std::string(buffer);
						eResult = it2.current->QueryStringAttribute("SwitchField", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS)
							field.SwitchField = std::string(buffer);
						eResult = it2.current->QueryStringAttribute("Terminator", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS)
							field.Terminator = std::string(buffer);
						eResult = it2.current->QueryUnsigned64Attribute("Length", &field.Length);
						eResult = it2.current->QueryUnsigned64Attribute("SwitchValue", &field.SwitchValue);
						eResult = it2.current->QueryBoolAttribute("IsLengthInBytes", &field.IsLengthInBytes);
						stype.Fields.push_back(field);
					}
					stype.NodeId = nameToNodeId.at(ModelOpcUa::QualifiedName_t{td.TargetNamespace, stype.Name});
					stype.BinaryNodeId = nameToBinaryNodeId.at(ModelOpcUa::QualifiedName_t{td.TargetNamespace, stype.Name});
					td.StructuredTypes.push_back(stype);
				}

				xml_element = xml_file.RootElement();
				if (xml_element == nullptr)	{
					LOG(ERROR) << "Error finding XMLRootElement of TypeDictionary " << dict.NodeId;
					continue;
				}

				firstNode = xml_element->FirstChildElement("opc:EnumeratedType");
				for (XMLIterator it = {.visitedElements = 0, .current = firstNode};
					 it.current != nullptr || (it.current != firstNode && it.visitedElements != 0);
					 it = {.visitedElements = it.visitedElements++, .current = it.current->NextSiblingElement("opc:EnumeratedType")})
				{
					TypeDictionary::EnumeratedType etype;
					eResult = it.current->QueryStringAttribute("Name", &buffer);
					if (eResult == tinyxml2::XML_SUCCESS) etype.Name = std::string(buffer);
					eResult = it.current->QueryUnsigned64Attribute("LengthInBits", &etype.LengthInBits);
					auto firstNodeInner = it.current->FirstChildElement("opc:EnumeratedValue");
					for (XMLIterator it2 = {.visitedElements = 0, .current = firstNodeInner};
						 it2.current != nullptr || (it2.current != firstNodeInner && it2.visitedElements != 0);
						 it2 = {.visitedElements = it2.visitedElements++, .current = it2.current->NextSiblingElement("opc:EnumeratedValue")})
					{
						TypeDictionary::EnumeratedValue evalue{};
						eResult = it2.current->QueryStringAttribute("Name", &buffer);
						if (eResult == tinyxml2::XML_SUCCESS) evalue.Name = std::string(buffer);
						eResult = it2.current->QueryInt64Attribute("Value", &evalue.Value);
						etype.EnumeratedValues.push_back(evalue);
					}
					auto id = TranslateBrowsePathToNodeId(Dashboard::NodeId_Enumeration, ModelOpcUa::QualifiedName_t{td.TargetNamespace, etype.Name});
					etype.NodeId = id;
					td.EnumeratedTypes.push_back(etype);
				}
				m_ptdv.push_back(td);
			}
		}

		OpcUaClient::~OpcUaClient()
		{
			m_tryConnecting = false;
			if (m_connectThread)
			{
				m_connectThread->join();
			}
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			m_subscr.deleteSubscription(m_pClient.get());
			disconnect();
		}

		bool OpcUaClient::disconnect()
		{
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			m_subscr.deleteSubscription(m_pClient.get());
			return (m_opcUaWrapper->SessionDisconnect(m_pClient.get(), UA_TRUE) != UA_STATUSCODE_GOOD) ? false : true;

		}

		void OpcUaClient::threadConnectExecution()
		{
			while (m_tryConnecting)
			{
				if (!m_isConnected)
				{
					this->connect();
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
		}
		std::list<ModelOpcUa::BrowseResult_t> OpcUaClient::Browse(
			ModelOpcUa::NodeId_t startNode,
			BrowseContext_t browseContext)
		{
			UA_BrowseDescription uaBrowseContext = getUaBrowseContext(browseContext);
			return BrowseWithContextAndFilter(startNode, uaBrowseContext);
		}

		std::list<ModelOpcUa::BrowseResult_t> OpcUaClient::BrowseWithResultTypeFilter(
			ModelOpcUa::NodeId_t startNode,
			BrowseContext_t browseContext,
			ModelOpcUa::NodeId_t typeDefinition)
		{
			UA_BrowseDescription uaBrowseContext = getUaBrowseContext(browseContext);
			open62541Cpp::UA_NodeId typeDefinitionUaNodeId = Converter::ModelNodeIdToUaNodeId(
											  typeDefinition,
											  m_uriToIndexCache)
											  .getNodeId();

			uaBrowseContext.nodeClassMask = nodeClassFromNodeId(typeDefinitionUaNodeId);
			auto filter = [&](const UA_ReferenceDescription &ref) {
				open62541Cpp::UA_NodeId browseTypeNodeId = open62541Cpp::UA_NodeId(ref.typeDefinition.nodeId);
				return isSameOrSubtype(typeDefinitionUaNodeId, browseTypeNodeId);
			};
			return BrowseWithContextAndFilter(startNode, uaBrowseContext, filter);
		}

		UA_NodeClass OpcUaClient::nodeClassFromNodeId(const open62541Cpp::UA_NodeId &typeDefinitionUaNodeId)
		{
			UA_NodeClass nodeClass = readNodeClass(typeDefinitionUaNodeId);

			switch (nodeClass)
			{
			case UA_NODECLASS_OBJECTTYPE:
			{
				return UA_NODECLASS_OBJECT;
				break;
			}
			case UA_NODECLASS_VARIABLETYPE:
			{
				return UA_NODECLASS_VARIABLE;
				break;
			}
			default:
				LOG(ERROR) << "Invalid NodeClass " << nodeClass
						   << " expect object or variable type for node "
						   << typeDefinitionUaNodeId.NodeId->identifier.string.data;
				throw Exceptions::UmatiException("Invalid NodeClass");
			}
		}

		std::list<ModelOpcUa::BrowseResult_t> OpcUaClient::BrowseWithContextAndFilter(
			const ModelOpcUa::NodeId_t &startNode,
			UA_BrowseDescription &browseContext,
			std::function<bool(const UA_ReferenceDescription &)> filter)
		{
			Converter::ModelNodeIdToUaNodeId conv = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache);
			open62541Cpp::UA_NodeId currentUaNodeId = conv.getNodeId();

			UA_ByteString continuationPoint;
			UA_ByteString_init(&continuationPoint);
			std::vector<UA_ReferenceDescription> referenceDescriptions;
			UA_BrowseResponse uaResult;

			checkConnection();
			{
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			uaResult = m_opcUaWrapper->SessionBrowse(m_pClient.get(), /*m_defaultServiceSettings,*/ currentUaNodeId, browseContext,
														  continuationPoint, referenceDescriptions);
			}

			ScopeExitGuard browseGuard([&]() {
			UA_BrowseDescription_clear(&browseContext);
			UA_BrowseResponse_clear(&uaResult);
			UA_ByteString_clear(&continuationPoint);
			});

			if (uaResult.resultsSize > 0 && UA_StatusCode_isBad(uaResult.results->statusCode))
			{
				LOG(ERROR) << "Bad return from browse with currentUaNodeId: "
						   << currentUaNodeId.NodeId->identifier.string.data
						   << " and ref id " << browseContext.referenceTypeId.identifier.string.data
						   << "Updating NamespaceCache...";
				updateNamespaceCache();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult.results->statusCode);
			}

			std::list<ModelOpcUa::BrowseResult_t> browseResult;
			ReferenceDescriptionsToBrowseResults(referenceDescriptions, browseResult, filter);

			handleContinuationPoint(continuationPoint);

			return browseResult;
		}

		void OpcUaClient::ReferenceDescriptionsToBrowseResults(
			const std::vector<UA_ReferenceDescription> &referenceDescriptions,
			std::list<ModelOpcUa::BrowseResult_t> &browseResult,
			std::function<bool(const UA_ReferenceDescription &)> filter)
		{
				for (UA_Int32 i = 0; i < referenceDescriptions.size(); i++)
			{
				if (!filter(referenceDescriptions.at(i)))
				{
				continue;
				}
					UA_ReferenceDescription tmp = referenceDescriptions.at(i);
					auto entry = ReferenceDescriptionToBrowseResult(tmp);
					browseResult.push_back(entry);
			}
		}

		void OpcUaClient::handleContinuationPoint(const UA_ByteString & /*continuationPoint*/)
		{
			// LOG(DEBUG) << "Handling continuation point not yet implemented";
		}

		ModelOpcUa::BrowseResult_t
		OpcUaClient::ReferenceDescriptionToBrowseResult(const UA_ReferenceDescription &referenceDescription)
		{
			ModelOpcUa::BrowseResult_t entry;

			open62541Cpp::UA_NodeId browseTypeUaNodeId;
			UA_NodeId_copy(&referenceDescription.typeDefinition.nodeId, browseTypeUaNodeId.NodeId);

			entry.NodeClass = Converter::UaNodeClassToModelNodeClass(referenceDescription.nodeClass).getNodeClass();
			entry.TypeDefinition = Converter::UaNodeIdToModelNodeId(browseTypeUaNodeId,m_indexToUriCache).getNodeId();

			open62541Cpp::UA_NodeId referenceDescriptionTmp;
			UA_NodeId_copy(&referenceDescription.nodeId.nodeId, referenceDescriptionTmp.NodeId);
			entry.NodeId = Converter::UaNodeIdToModelNodeId(referenceDescriptionTmp, m_indexToUriCache).getNodeId();

			open62541Cpp::UA_NodeId referenceTypeUaNodeId;
			UA_NodeId_copy(&referenceDescription.referenceTypeId, referenceTypeUaNodeId.NodeId);

			auto referenceTypeModelNodeId = Converter::UaNodeIdToModelNodeId(referenceTypeUaNodeId,m_indexToUriCache).getNodeId();

			entry.ReferenceTypeId = referenceTypeModelNodeId;
			open62541Cpp::UA_QualifiedName browseName(referenceDescription.browseName.namespaceIndex,
														std::string((char*)referenceDescription.browseName.name.data,
														referenceDescription.browseName.name.length));

			entry.BrowseName = Converter::UaQualifiedNameToModelQualifiedName(browseName,m_indexToUriCache).getQualifiedName();

			return entry;
		}

		UA_BrowseDescription OpcUaClient::prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId)
		{
			auto referenceTypeUaNodeId = Converter::ModelNodeIdToUaNodeId(std::move(referenceTypeId),
																		  m_uriToIndexCache)
											 .getNodeId();
			UA_BrowseDescription browseContext;
            UA_BrowseDescription_init(&browseContext);
			browseContext.browseDirection = UA_BROWSEDIRECTION_FORWARD;
			browseContext.includeSubtypes = UA_TRUE;
			browseContext.nodeClassMask = 0; // ALL
			if (UA_NodeId_isNull(referenceTypeUaNodeId.NodeId))
			{
				UA_NodeId_copy(referenceTypeUaNodeId.NodeId, &browseContext.referenceTypeId);
			}
			browseContext.resultMask =
				UA_BROWSERESULTMASK_BROWSENAME + UA_BROWSERESULTMASK_TYPEDEFINITION + UA_BROWSERESULTMASK_NODECLASS + UA_BROWSERESULTMASK_REFERENCETYPEID;
			return browseContext;
		}
		UA_BrowseDescription OpcUaClient::getUaBrowseContext(const IDashboardDataClient::BrowseContext_t &browseContext)
		{
			UA_BrowseDescription ret;
            UA_BrowseDescription_init(&ret);
			ret.browseDirection = (decltype(ret.browseDirection))browseContext.browseDirection;
			ret.includeSubtypes = browseContext.includeSubtypes ?  UA_TRUE : UA_FALSE;
			ret.nodeClassMask = (decltype(ret.nodeClassMask))browseContext.nodeClassMask;

			if(!browseContext.referenceTypeId.isNull())
			{
				ret.referenceTypeId = *Converter::ModelNodeIdToUaNodeId(browseContext.referenceTypeId, m_uriToIndexCache).getNodeId().NodeId;
			}else{
				UA_NodeId_clear(&ret.referenceTypeId);
			}
			ret.resultMask = (decltype(ret.resultMask))browseContext.resultMask;
			return ret;
		}

		ModelOpcUa::NodeId_t
		OpcUaClient::TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode,
												 ModelOpcUa::QualifiedName_t browseName)
		{
			checkConnection();

			if (browseName.isNull())
			{
				LOG(ERROR) << "browseName is NULL";
				throw std::invalid_argument("browseName is NULL");
			}

			if (startNode.isNull())
			{
				LOG(ERROR) << "startNode is NULL";
				throw std::invalid_argument("startNode is NULL");
			}

			auto currentUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode,
																  m_uriToIndexCache)
																  .getNodeId();

			auto uaBrowseName = Converter::ModelQualifiedNameToUaQualifiedName(browseName,
																			   m_uriToIndexCache)
																			   .detach();

			UA_BrowsePath uaBrowsePaths;
			UA_BrowsePath_init(&uaBrowsePaths);
			uaBrowsePaths.relativePath.elementsSize = 1;
			uaBrowsePaths.relativePath.elements = UA_RelativePathElement_new();
            uaBrowsePaths.relativePath.elements->includeSubtypes = UA_TRUE;
			uaBrowsePaths.relativePath.elements->isInverse = UA_FALSE;
			uaBrowsePaths.relativePath.elements->referenceTypeId.identifier.numeric = UA_NS0ID_HIERARCHICALREFERENCES;
			uaBrowsePaths.relativePath.elements->targetName = uaBrowseName;
			UA_NodeId_copy(currentUaNodeId.NodeId, &uaBrowsePaths.startingNode);

			UA_BrowsePathResult uaBrowsePathResults;
			UA_DiagnosticInfo uaDiagnosticInfos;
			UA_StatusCode uaResult;

			ScopeExitGuard browseGuard([&]() {
				UA_BrowsePathResult_clear(&uaBrowsePathResults);
				UA_BrowsePath_clear(&uaBrowsePaths);
			});
			{
            std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			uaResult = m_opcUaWrapper->SessionTranslateBrowsePathsToNodeIds(
				m_pClient.get(),
				uaBrowsePaths,
				uaBrowsePathResults,
				uaDiagnosticInfos);
			}
            if (UA_StatusCode_isBad(uaResult))
			{
                if (uaResult != UA_STATUSCODE_BADNOMATCH) {
                    LOG(ERROR) << "TranslateBrowsePathToNodeId failed for node: '" << static_cast<std::string>(startNode)
						   << "' with " << UA_StatusCode_name(uaResult) << "(BrowsePath: "
						   << static_cast<std::string>(browseName) << ")";
				}
				 if(uaResult == UA_STATUSCODE_BADNODEIDUNKNOWN){
					LOG(INFO) << "Updating NamespaceCache because of " << UA_StatusCode_name(uaResult);
					updateNamespaceCache();
				}
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}
			if (uaBrowsePathResults.targetsSize != 1)
			{
				LOG(ERROR) << "Expect 1 browseResult, got " << uaBrowsePathResults.targetsSize << " for node: '"
						   << static_cast<std::string>(startNode)
						   << "' with " << uaResult << "(BrowsePath: "
						   << static_cast<std::string>(browseName) << ")";
				throw Exceptions::UmatiException("BrowseResult length mismatch.");
			}

			UA_StatusCode uaResultElement(uaBrowsePathResults.statusCode);

			if (UA_StatusCode_isBad(uaResultElement))
			{
				std::stringstream ss;
				ss << "Element returned bad status code: " << uaResultElement
						   << " for node: '"
						   << static_cast<std::string>(startNode) << "' with " << uaResult
						   << " (BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement, ss.str());
			}

			if (uaBrowsePathResults.targetsSize != 1)
			{
				LOG(WARNING) << "Continuing with index 0 - expected one target, got "
							 << uaBrowsePathResults.targetsSize
							 << " for node: '" << static_cast<std::string>(startNode) << "' with "
							 << uaResult << "(BrowsePath: " << static_cast<std::string>(browseName)
							 << ")";
				for (int target_id = 0; target_id < uaBrowsePathResults.targetsSize; target_id++)
				{
					try
					{
						open62541Cpp::UA_NodeId _targetNodeId(uaBrowsePathResults.targets->targetId.nodeId);
						auto nodeId = Converter::UaNodeIdToModelNodeId(_targetNodeId, m_indexToUriCache).getNodeId();
						LOG(WARNING) << "Target " << target_id << " | id: " << nodeId.Uri << ";" << nodeId.Id;
					}
					catch (std::exception &ex)
					{
						LOG(ERROR) << "error  getting nodeId " << ex.what();
					}
				}
			}

			open62541Cpp::UA_NodeId targetNodeId(uaBrowsePathResults.targets->targetId.nodeId);

			return Converter::UaNodeIdToModelNodeId(targetNodeId, m_indexToUriCache).getNodeId();
			}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>
		OpcUaClient::Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback)
		{
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);

			try{
				return m_opcUaWrapper->SubscriptionSubscribe(m_pClient.get(), nodeId, callback);
			}catch(std::exception &ex){
				LOG(ERROR) << "Updating Namespace cache after exception: "<< ex.what();
				updateNamespaceCache();
			}
			return nullptr;
		}

		void OpcUaClient::Unsubscribe(std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles){

			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			m_opcUaWrapper->SubscriptionUnsubscribe(m_pClient.get(), monItemIds, clientHandles);
		}

		std::vector<nlohmann::json> OpcUaClient::ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds)
		{
			return readValues2(modelNodeIds);
		}

		std::vector<nlohmann::json> OpcUaClient::readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds)
		{

			std::vector<nlohmann::json> readValues;

			UA_DiagnosticInfo info;
			UA_DiagnosticInfo_init(&info);

			const size_t readValueSize = modelNodeIds.size();
			UA_ReadValueId *readValueId = (UA_ReadValueId *) UA_Array_new(readValueSize, &UA_TYPES[UA_TYPES_READVALUEID]);


			auto index = 0;
			for (const auto &modelNodeId : modelNodeIds)
			{
				open62541Cpp::UA_NodeId nodeId = Converter::ModelNodeIdToUaNodeId(modelNodeId, m_uriToIndexCache).getNodeId();

				readValueId[index].attributeId = UA_ATTRIBUTEID_VALUE;
				UA_NodeId_copy(nodeId.NodeId, &readValueId[index].nodeId);
				index++;
			}

			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			auto ret = m_opcUaWrapper->SessionRead(m_pClient.get(),0.0,UA_TIMESTAMPSTORETURN_BOTH, readValueId,  readValueSize, info);

			if (UA_StatusCode_isBad(ret.results->status))
			{
				std::stringstream ss;
				ss << "Received non good status for reading: " << UA_StatusCode_name(ret.results->status);
				LOG(ERROR) << ss.str();
				UA_Array_delete(readValueId, readValueSize, &UA_TYPES[UA_TYPES_READVALUEID]);
				UA_ReadResponse_clear(&ret);

				throw Exceptions::OpcUaException(ss.str());
			}
			else
			{
				for(int i = 0; i < ret.resultsSize; i++){

					auto valu = Converter::UaDataValueToJsonValue(ret.results[i], false);
					auto val = valu.getValue();
					readValues.push_back(val);
				}
			}

			for(int i = 0; i < readValueSize; i++){
				UA_NodeId_clear(&readValueId[i].nodeId);
			}
			UA_Array_delete(readValueId, readValueSize, &UA_TYPES[UA_TYPES_READVALUEID]);
			UA_ReadResponse_clear(&ret);

			return readValues;
		}

		std::vector<std::string> OpcUaClient::Namespaces()
		{
			std::vector<std::string> ret;
			for(auto &ns : m_indexToUriCache)
			{
				ret.push_back(ns.second);
			}
			return ret;
		}

		bool OpcUaClient::VerifyConnection() {
			std::lock_guard<std::recursive_mutex> l(m_clientMutex);
			UA_NodeClass nodeClass = UA_NodeClass::UA_NODECLASS_OBJECT;
			auto status = UA_Client_readNodeClassAttribute(m_pClient.get(), UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_NAMESPACEARRAY), &nodeClass);
			if(status != UA_STATUSCODE_GOOD) {
				LOG(WARNING) << "Verify connection failed. Got status code: " << UA_StatusCode_name(status);
				return false;
			}
			if(nodeClass != UA_NodeClass::UA_NODECLASS_VARIABLE) {
				LOG(WARNING) << "Getting NodeClass failed. Got NodeClass: " << (status);
				return false;
			}

			return true;
		}

	} // namespace OpcUa
} // namespace Umati
