#include "OpcUaClient.hpp"

#include "SetupSecurity.hpp"

#include <Exceptions/ClientNotConnected.hpp>
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Converter/ModelQualifiedNameToUaQualifiedName.hpp"
#include "Converter/UaQualifiedNameToModelQualifiedName.hpp"
#include "Converter/UaNodeClassToModelNodeClass.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"

namespace Umati
{

	namespace OpcUa
	{

		int OpcUaClient::PlatformLayerInitialized = 0;

		OpcUaClient::OpcUaClient(std::string serverURI, std::string Username, std::string Password,
								 std::uint8_t security, std::vector<std::string> expectedObjectTypeNamespaces,
								 std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper)
			: m_serverUri(std::move(serverURI)), m_username(std::move(Username)), m_password(std::move(Password)),
			  m_security(static_cast<UA_MessageSecurityMode>(security)),
			 m_subscr(m_uriToIndexCache, m_indexToUriCache)
			 
		{
			client = UA_Client_new();
			UA_ClientConfig *config = UA_Client_getConfig(client);
			config->timeout = 10000;
			UA_ClientConfig_setDefault(config);
			m_opcUaWrapper = std::move(opcUaWrapper);
			m_opcUaWrapper->setSubscription(&m_subscr);

			if (++PlatformLayerInitialized == 1)
			{
				//TODO find according datatype or funciton
			//	UaPlatformLayer::init();
			}

			m_tryConnecting = true;
			// Try connecting at least once
			this->connect();
			m_connectThread = std::make_shared<std::thread>([this]() { this->threadConnectExecution(); });
		}

		bool OpcUaClient::connect()
		{
			open62541Cpp::UA_String sURL(m_serverUri.c_str());
			UA_StatusCode  result;

			UA_EndpointDescription* endpointDescriptions = NULL;
			size_t endpointArraySize = 0;

			UA_ApplicationDescription applicationDescriptions;
			//TODO use security.. -> UaClientSdk::SessionSecurityInfo sessionSecurityInfo
			SetupSecurity::setupSecurity();


			result = m_opcUaWrapper->DiscoveryGetEndpoints(client,
															&sURL, 
															&endpointArraySize,
															&endpointDescriptions); 
			if (result != UA_STATUSCODE_GOOD)	
			{
				LOG(ERROR) << UA_StatusCode_name(result);
				UA_Client_delete(client);
				return false;
			}


			struct
			{
				open62541Cpp::UA_String url;
				UA_ByteString serverCertificate;
				open62541Cpp::UA_String securityPolicy;
				UA_Int32 securityMode{};
			} desiredEndpoint;

			//TODO security 
			// auto desiredSecurity = m_security;
			for (UA_Int32 iEndpoint = 0; iEndpoint < endpointArraySize; iEndpoint++)
			{

			/*	if (endpointDescriptions[iEndpoint].securityMode != desiredSecurity)
				{
					continue;
				}
			*/

				desiredEndpoint.url.String->length = endpointDescriptions[iEndpoint].endpointUrl.length;
				desiredEndpoint.url.String->data = endpointDescriptions[iEndpoint].endpointUrl.data;

				LOG(INFO) << "desiredEndpoint.url: " << desiredEndpoint.url << std::endl;
				//sessionSecurityInfo.serverCertificate = endpointDescriptions[iEndpoint].ServerCertificate;
				//sessionSecurityInfo.sSecurityPolicy = endpointDescriptions[iEndpoint].SecurityPolicyUri;
				//sessionSecurityInfo.messageSecurityMode = static_cast<OpcUa_MessageSecurityMode>(endpointDescriptions[iEndpoint].SecurityMode);
				break;
			} 
		
			if (desiredEndpoint.url.String->length == 0)
			{
				LOG(ERROR) << "Could not find endpoint without encryption." << std::endl;
				UA_Client_delete(client);
				return false;
			}
	
			///\todo handle security

		    //TODO security 
			/*sessionSecurityInfo.doServerCertificateVerify = OpcUa_False;
			sessionSecurityInfo.disableErrorCertificateHostNameInvalid = OpcUa_True;
			sessionSecurityInfo.disableApplicationUriCheck = OpcUa_True;

			if (!m_username.empty() && !m_password.empty())
			{
				sessionSecurityInfo.setUserPasswordUserIdentity(m_username.c_str(), m_password.c_str());
			}
			*/
			m_opcUaWrapper->GetNewSession(m_pSession);

			//VERIFY sessionConnectInfo is not used in SessionConnect.
			UA_CreateSessionRequest sessionConnectInfo;
			sessionConnectInfo = prepareSessionConnectInfo(sessionConnectInfo);
			//TODO security info. 
			result = m_opcUaWrapper->SessionConnect(client,sURL, sessionConnectInfo /*sessionSecurityInfo, this*/);
			if (result != UA_STATUSCODE_GOOD)
			{
				LOG(ERROR) << "Connecting failed in OPC UA Data Client: " << UA_StatusCode_name(result) << std::endl;
				//VERIFY ConnectionStatusChanged wont be called otherwise
				connectionStatusChanged(0,UA_SERVERSTATE_FAILED);
				UA_Client_delete(client);
				return false;	
			} 
			connectionStatusChanged(0,UA_SERVERSTATE_RUNNING);

			return true;
		}
		//VERIFY UA_CreateSessionRequest or UA_ApplicationDescription?
		UA_CreateSessionRequest &
		OpcUaClient::prepareSessionConnectInfo(UA_CreateSessionRequest &sessionConnectInfo)
		{
			sessionConnectInfo.clientDescription.applicationName = UA_LOCALIZEDTEXT("en-US","KonI4.0 OPC UA Data Client");
			sessionConnectInfo.clientDescription.applicationUri = UA_String_fromChars("http://dashboard.umati.app/OPCUA_DataClient");
			sessionConnectInfo.clientDescription.productUri = UA_String_fromChars("KonI40OpcUaClient_Product");
			sessionConnectInfo.sessionName = UA_String_fromChars("DefaultSession");
			return sessionConnectInfo;
		}

		void OpcUaClient::on_connected()
		{
			updateNamespaceCache();
			m_opcUaWrapper->SubscriptionCreateSubscription(client, m_pSession);
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
			auto uaResult = UA_Client_readBrowseNameAttribute(client, *nodeId.NodeId, &resultname);

			if (UA_StatusCode_isBad(uaResult))
			{
				LOG(ERROR) << "readNodeClass failed for node: '" << nodeId.NodeId->identifier.string.data
						   << "' with " << UA_StatusCode_name(uaResult);
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			return _nodeId.Uri + ";" + std::string((char *)resultname.name.data,resultname.name.length);
		}

		UA_NodeClass OpcUaClient::readNodeClass(const open62541Cpp::UA_NodeId &nodeId)
		{
			checkConnection();

			UA_NodeClass returnClass;
			UA_NodeClass_init(&returnClass);

			auto uaResult = UA_Client_readNodeClassAttribute(client, *nodeId.NodeId, &returnClass);
			if (UA_StatusCode_isBad(uaResult))
			{
				LOG(ERROR) << "readNodeClass failed for node: '" << nodeId.NodeId->identifier.string.data
						<< "' with " << UA_StatusCode_name(uaResult);
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}
				
			return returnClass;
		}

		void OpcUaClient::checkConnection()
		{
			if (!this->m_isConnected || !m_opcUaWrapper->SessionIsConnected())
			{
				throw Exceptions::ClientNotConnected("Need connected client.");
			}
		}

		open62541Cpp::UA_NodeId OpcUaClient::browseSuperType(const open62541Cpp::UA_NodeId &typeNodeId)
		{
			checkConnection();
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
			auto uaResult = m_opcUaWrapper->SessionBrowse(client, /*m_defaultServiceSettings,*/ typeNodeId, browseContext,
														  continuationPoint, referenceDescriptions);
			
			if (uaResult.results->statusCode != UA_STATUSCODE_GOOD)
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.results->statusCode;
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult.results->statusCode);
			}

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
			UA_ExpandedNodeId_copy(&referenceDescriptions.at(0).typeDefinition,&retExpandedNodeId);

			return open62541Cpp::UA_NodeId(retExpandedNodeId.nodeId);
		}

		bool
		OpcUaClient::isSameOrSubtype(const open62541Cpp::UA_NodeId &expectedType, const open62541Cpp::UA_NodeId &checkType, std::size_t maxDepth)
		{
			if (UA_NodeId_isNull(checkType.NodeId))
			{
				return false;
			}

			if (UA_NodeId_equal(expectedType.NodeId,checkType.NodeId))
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

		void OpcUaClient::initializeNamespaceCache()
		{
			m_opcUaWrapper->SessionUpdateNamespaceTable(client);

			m_uriToIndexCache.clear();
			m_indexToUriCache.clear();
		}

		void OpcUaClient::updateNamespaceCache()
		{
			initializeNamespaceCache();

	    	auto uaNamespaces = m_opcUaWrapper->SessionGetNamespaceTable();
		
			fillNamespaceCache(uaNamespaces);
		}
		void OpcUaClient::fillNamespaceCache(const std::vector<std::string> &uaNamespaces)
		{
			for(size_t i = 0; i < uaNamespaces.size(); ++i){

				std::string namespaceURI = uaNamespaces.at(i);
				
				if ( m_uriToIndexCache.find(namespaceURI) == m_uriToIndexCache.end() ) {
				    m_uriToIndexCache[namespaceURI] = static_cast<uint16_t>(i);
					m_indexToUriCache[static_cast<uint16_t>(i)] = namespaceURI;
				} else {
				    LOG(INFO) << "Namepsace already in cache";
				}
		
				LOG(INFO) << "index: " << std::to_string(i) << ", namespaceURI: " << namespaceURI;
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

			auto uaResult2 = m_opcUaWrapper->SessionBrowse(client, /*m_defaultServiceSettings,*/ uaNodeId,
														   browseContext2,
														   continuationPoint, referenceDescriptions);
			if (uaResult2.results->statusCode != UA_STATUSCODE_GOOD)
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult2.results->statusCode << "for nodeId"
						   << uaNodeId.NodeId->identifier.string.data;
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult2.results->statusCode);
			}
			
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
			//FIXME find correct type
		/*	case UaClientSdk::UaClient::ConnectionWarningWatchdogTimeout:
				LOG(ERROR) << "ConnectionWarningWatchdogTimeout." << std::endl;
				break;
			case UaClientSdk::UaClient::ConnectionErrorApiReconnect:
				LOG(ERROR) << "ConnectionErrorApiReconnect." << std::endl;
				break;
		*/	
			case UA_SERVERSTATE_SHUTDOWN:
				LOG(ERROR) << "ServerShutdown." << std::endl;
				break;
			case UA_SESSIONSTATE_CREATED:
				LOG(ERROR) << "NewSessionCreated." << std::endl;
				break;
			
			}
		}
		

		OpcUaClient::~OpcUaClient()
		{
			// Destroy all sessions before UaPlatformLayer::cleanup(); is called!
			m_tryConnecting = false;
			if (m_connectThread)
			{
				m_connectThread->join();
			}

			m_pSession = nullptr;

			if (--PlatformLayerInitialized == 0)
			{
				//TODO cleanup function?
				//UaPlatformLayer::cleanup();
				UA_Client_delete(client);
			}
		}

		bool OpcUaClient::disconnect()
		{
			if (m_pSession)
			{
				//Subscr.deleteSubscription(m_pSession);
				//VERIFY do we need ServiceSettings object?
				//UaClientSdk::ServiceSettings servsettings;
				
				return (m_opcUaWrapper->SessionDisconnect(client, UA_TRUE) != UA_STATUSCODE_GOOD) ? false : true;
			}
			return true;
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
			auto typeDefinitionUaNodeId = (open62541Cpp::UA_NodeId) Converter::ModelNodeIdToUaNodeId(
											  typeDefinition,
											  m_uriToIndexCache)
											  .getNodeId();

			uaBrowseContext.nodeClassMask = nodeClassFromNodeId(typeDefinitionUaNodeId);
			auto filter = [&](const UA_ReferenceDescription &ref) {
				const open62541Cpp::UA_NodeId browseTypeNodeId;
				UA_NodeId_copy(&ref.typeDefinition.nodeId,browseTypeNodeId.NodeId);
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
			checkConnection();
			Converter::ModelNodeIdToUaNodeId conv = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache);
			open62541Cpp::UA_NodeId startUaNodeId = conv.getNodeId();

			UA_ByteString continuationPoint;
			std::vector<UA_ReferenceDescription> referenceDescriptions;

			UA_BrowseResponse uaResult = m_opcUaWrapper->SessionBrowse(client, /*m_defaultServiceSettings,*/ startUaNodeId, browseContext,
														  continuationPoint, referenceDescriptions);

		
			//referenceDescriptions = uaResult.results->references;
		    if (UA_StatusCode_isBad(uaResult.results->statusCode))
			 {
				LOG(ERROR) << "Bad return from browse: " << uaResult.results->references->browseName.name.data << ", with startUaNodeId "
						   << startUaNodeId.NodeId->identifier.string.data
						   << " and ref id " << browseContext.referenceTypeId.identifier.string.data;
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
			//VERIFY use of vector
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
			LOG(DEBUG) << "Handling continuation point not yet implemented";
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

			auto startUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode,
																  m_uriToIndexCache)
																  .getNodeId();

			auto uaBrowseName = Converter::ModelQualifiedNameToUaQualifiedName(browseName,
																			   m_uriToIndexCache)
																			   .getQualifiedName();
			// LOG(INFO) << "translateBrowsePathToNodeId: start from " << startUaNodeId.toString().toUtf8() << " and search " << uaBrowseName.toString().toUtf8();

			/*VERIFY creation of elem. removed all [0] of uaBrowsePathElements and uaBrowsePaths
			 * uaBrowsePathElements was a container type with size 1. See next comment
			 * thats why i did not use an container type around UA_RelativePathElement and set the 
			 * elemtsSize to 1
			 */
			UA_RelativePathElement uaBrowsePathElements;
			UA_RelativePathElement_init(&uaBrowsePathElements);
			//uaBrowsePathElements.create(1);
			uaBrowsePathElements.includeSubtypes = UA_TRUE;
			uaBrowsePathElements.isInverse = UA_FALSE;
			uaBrowsePathElements.referenceTypeId.identifier.numeric = UA_NS0ID_HIERARCHICALREFERENCES;
			UA_QualifiedName_copy(&uaBrowseName,&uaBrowsePathElements.targetName);

			UA_BrowsePath uaBrowsePaths;
			UA_BrowsePath_init(&uaBrowsePaths);
			//uaBrowsePaths.create(1);
			uaBrowsePaths.relativePath.elementsSize = 1;

			//UA_RelativePathElement_copy(&uaBrowsePathElements, uaBrowsePaths.relativePath.elements);
			uaBrowsePaths.relativePath.elements = &uaBrowsePathElements;
			//FIXME double assignment? Using Copy functions will lead to SEGV
			uaBrowsePaths.relativePath.elements->targetName = uaBrowsePathElements.targetName;
			uaBrowsePaths.relativePath.elements->referenceTypeId = uaBrowsePathElements.referenceTypeId;
			UA_NodeId_copy(startUaNodeId.NodeId, &uaBrowsePaths.startingNode);

			UA_BrowsePathResult uaBrowsePathResults;
			UA_DiagnosticInfo uaDiagnosticInfos;

			auto uaResult = m_opcUaWrapper->SessionTranslateBrowsePathsToNodeIds(
				client,
				/*m_defaultServiceSettings,*/
				uaBrowsePaths,
				uaBrowsePathResults,
				uaDiagnosticInfos);

			if (UA_StatusCode_isBad(uaResult))
			{
				LOG(ERROR) << "TranslateBrowsePathToNodeId failed for node: '" << static_cast<std::string>(startNode)
						   << "' with " << uaResult << "(BrowsePath: "
						   << static_cast<std::string>(browseName) << ")";
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
			
			if (uaResultElement != UA_STATUSCODE_GOOD)
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
		OpcUaClient::Subscribe(UA_Client *client ,ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback)
		{
			return m_opcUaWrapper->SubscriptionSubscribe(client, nodeId, callback);
		}
	
		std::vector<nlohmann::json> OpcUaClient::ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds)
		{
			std::vector<nlohmann::json> ret;
			std::vector<UA_DataValue> readValues = readValues2(modelNodeIds);

			for (uint i = 0; i < readValues.size(); ++i) 
			{
				auto value = readValues[i];
				auto valu = Converter::UaDataValueToJsonValue(value, false);
				auto val = valu.getValue();
				ret.push_back(val);
			}
			return ret;
		}

		std::vector<UA_DataValue> OpcUaClient::readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds)
		{

			std::vector<UA_DataValue> readValues;
			UA_Variant tmpVariant;
			UA_Variant_init(&tmpVariant);

			UA_DataValue tmpReadValue;
			UA_DataValue_init(&tmpReadValue);

			for (const auto &modelNodeId : modelNodeIds)
			{

				open62541Cpp::UA_NodeId nodeId = Converter::ModelNodeIdToUaNodeId(modelNodeId, m_uriToIndexCache).getNodeId();
				tmpReadValue.status = UA_Client_readValueAttribute(client, *nodeId.NodeId, &tmpVariant);
				tmpReadValue.value = tmpVariant;
				tmpReadValue.hasStatus = UA_TRUE;
				tmpReadValue.hasValue = UA_TRUE;

				if (UA_StatusCode_isBad(tmpReadValue.status))
				{
					LOG(ERROR) << "Received non good status for read: " << tmpReadValue.status;
					std::stringstream ss;
					ss << "Received non good status  for read: " << tmpReadValue.status;
					throw Exceptions::OpcUaException(ss.str());
				}
				else
				{
					readValues.push_back(tmpReadValue);
				}
				
			}

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

	} // namespace OpcUa
} // namespace Umati