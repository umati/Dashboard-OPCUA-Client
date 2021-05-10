#include "OpcUaClient.hpp"

#include "SetupSecurity.hpp"

#include "uaplatformlayer.h"

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
			  m_security(static_cast<OpcUa_MessageSecurityMode>(security)),
			  m_subscr(m_uriToIndexCache, m_indexToUriCache)
		{
			m_defaultServiceSettings.callTimeout = 10000;
			m_opcUaWrapper = std::move(opcUaWrapper);
			m_opcUaWrapper->setSubscription(&m_subscr);

			if (++PlatformLayerInitialized == 1)
			{
				UaPlatformLayer::init();
			}

			m_tryConnecting = true;
			// Try connecting at least once
			this->connect();
			m_connectThread = std::make_shared<std::thread>([this]() { this->threadConnectExecution(); });
		}

		bool OpcUaClient::connect()
		{
			UaString sURL(m_serverUri.c_str());
			UaStatus result;

			UaClientSdk::SessionSecurityInfo sessionSecurityInfo;
			UaClientSdk::ServiceSettings serviceSettings;
			UaEndpointDescriptions endpointDescriptions;
			UaApplicationDescriptions applicationDescriptions;
			SetupSecurity::setupSecurity(&sessionSecurityInfo);

			result = m_opcUaWrapper->DiscoveryGetEndpoints(serviceSettings, sURL, sessionSecurityInfo,
														   endpointDescriptions);
			if (result.isBad())
			{
				LOG(ERROR) << result.toString().toUtf8();
				return false;
			}

			struct
			{
				UaString url;
				UaByteString serverCertificate;
				UaString securityPolicy;
				OpcUa_UInt32 securityMode{};
			} desiredEndpoint;

			auto desiredSecurity = m_security;
			for (OpcUa_UInt32 iEndpoint = 0; iEndpoint < endpointDescriptions.length(); iEndpoint++)
			{

				if (endpointDescriptions[iEndpoint].SecurityMode != desiredSecurity)
				{
					continue;
				}
				desiredEndpoint.url = UaString(endpointDescriptions[iEndpoint].EndpointUrl);
				LOG(INFO) << "desiredEndpoint.url: " << desiredEndpoint.url.toUtf8() << std::endl;
				sessionSecurityInfo.serverCertificate = endpointDescriptions[iEndpoint].ServerCertificate;
				sessionSecurityInfo.sSecurityPolicy = endpointDescriptions[iEndpoint].SecurityPolicyUri;
				sessionSecurityInfo.messageSecurityMode = static_cast<OpcUa_MessageSecurityMode>(endpointDescriptions[iEndpoint].SecurityMode);
				break;
			}

			if (desiredEndpoint.url.isEmpty())
			{
				LOG(ERROR) << "Could not find endpoint without encryption." << std::endl;
				return false;
			}

			///\todo handle security
			sessionSecurityInfo.doServerCertificateVerify = OpcUa_False;
			sessionSecurityInfo.disableErrorCertificateHostNameInvalid = OpcUa_True;
			sessionSecurityInfo.disableApplicationUriCheck = OpcUa_True;

			if (!m_username.empty() && !m_password.empty())
			{
				sessionSecurityInfo.setUserPasswordUserIdentity(m_username.c_str(), m_password.c_str());
			}

			m_opcUaWrapper->GetNewSession(m_pSession);

			UaClientSdk::SessionConnectInfo sessionConnectInfo;
			sessionConnectInfo = prepareSessionConnectInfo(sessionConnectInfo);

			result = m_opcUaWrapper->SessionConnect(sURL, sessionConnectInfo, sessionSecurityInfo, this);

			if (!result.isGood())
			{
				LOG(ERROR) << "Connecting failed in OPC UA Data Client: " << result.toString().toUtf8() << std::endl;
				return false;
			}

			return true;
		}

		UaClientSdk::SessionConnectInfo &
		OpcUaClient::prepareSessionConnectInfo(UaClientSdk::SessionConnectInfo &sessionConnectInfo)
		{
			sessionConnectInfo.sApplicationName = "KonI4.0 OPC UA Data Client";
			sessionConnectInfo.sApplicationUri = "http://dashboard.umati.app/OPCUA_DataClient";
			sessionConnectInfo.sProductUri = "KonI40OpcUaClient_Product";
			sessionConnectInfo.sSessionName = "DefaultSession";
			return sessionConnectInfo;
		}

		void OpcUaClient::on_connected()
		{
			updateNamespaceCache();
			m_opcUaWrapper->SubscriptionCreateSubscription(m_pSession);
		}

		std::string OpcUaClient::getTypeName(const ModelOpcUa::NodeId_t &nodeId)
		{
			return readNodeBrowseName(nodeId);
		}

		std::string OpcUaClient::readNodeBrowseName(const ModelOpcUa::NodeId_t &_nodeId)
		{
			auto nodeId = Converter::ModelNodeIdToUaNodeId(_nodeId, m_uriToIndexCache).getNodeId();

			checkConnection();

			UaReadValueIds readValueIds;
			readValueIds.create(1);
			nodeId.copyTo(&readValueIds[0].NodeId);
			readValueIds[0].AttributeId = OpcUa_Attributes_BrowseName;

			UaDataValues readResult;

			UaDiagnosticInfos diagInfo;

			auto uaResult = m_opcUaWrapper->SessionRead(
				m_defaultServiceSettings,
				100.0,
				OpcUa_TimestampsToReturn_Neither,
				readValueIds,
				readResult,
				diagInfo);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "readNodeClass failed for node: '" << nodeId.toXmlString().toUtf8()
						   << "' with " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (readResult.length() != 1)
			{
				LOG(ERROR) << "readResult.length() expect 1  got:" << readResult.length();
				throw Exceptions::UmatiException("Length mismatch");
			}

			UaStatusCode uaResultElement(readResult[0].StatusCode);
			if (uaResultElement.isBad())
			{
				LOG(WARNING) << "Bad value status code failed for node: '" << nodeId.toFullString().toUtf8()
							 << "' with " << uaResultElement.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement);
			}

			UaVariant value(readResult[0].Value);
			if (value.type() != OpcUaType_QualifiedName)
			{
				LOG(ERROR) << "Expect Type Int32, got '" << value.type();
				throw Exceptions::UmatiException("Type mismatch");
			}

			return _nodeId.Uri + ";" + value.toString().toUtf8();
		}

		OpcUa_NodeClass OpcUaClient::readNodeClass(const UaNodeId &nodeId)
		{
			checkConnection();

			UaReadValueIds readValueIds;
			readValueIds.create(1);
			nodeId.copyTo(&readValueIds[0].NodeId);
			readValueIds[0].AttributeId = OpcUa_Attributes_NodeClass;

			UaDataValues readResult;

			UaDiagnosticInfos diagInfo;

			auto uaResult = m_opcUaWrapper->SessionRead(
				m_defaultServiceSettings,
				100.0,
				OpcUa_TimestampsToReturn_Neither,
				readValueIds,
				readResult,
				diagInfo);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "readNodeClass failed for node: '" << nodeId.toXmlString().toUtf8()
						   << "' with " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (readResult.length() != 1)
			{
				LOG(ERROR) << "readResult.length() expect 1  got:" << readResult.length();
				throw Exceptions::UmatiException("Length mismatch");
			}

			UaStatusCode uaResultElement(readResult[0].StatusCode);
			if (uaResultElement.isBad())
			{
				LOG(WARNING) << "Bad value status code failed for node: '" << nodeId.toFullString().toUtf8()
							 << "' with " << uaResultElement.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement);
			}

			UaVariant value(readResult[0].Value);
			if (value.type() != OpcUaType_Int32)
			{
				LOG(ERROR) << "Expect Type Int32, got '" << value.type();
				throw Exceptions::UmatiException("Type mismatch");
			}

			OpcUa_Int32 nodeClass;
			value.toInt32(nodeClass);

			return static_cast<OpcUa_NodeClass>(nodeClass);
		}

		void OpcUaClient::checkConnection()
		{
			if (!this->m_isConnected || !m_opcUaWrapper->SessionIsConnected())
			{
				throw Exceptions::ClientNotConnected("Need connected client.");
			}
		}

		OpcUa_ExpandedNodeId OpcUaClient::browseTypeDefinition(const UaNodeId &nodeId) {
			checkConnection();

			auto referenceTypeUaNodeId = UaNodeId(OpcUaId_HasTypeDefinition);
			UaClientSdk::BrowseContext browseContext;
			browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
			browseContext.includeSubtype = OpcUa_True;
			browseContext.maxReferencesToReturn = 0;
			browseContext.nodeClassMask = 0; // ALL
			browseContext.referenceTypeId = referenceTypeUaNodeId;

			UaByteString continuationPoint;
			UaReferenceDescriptions referenceDescriptions;
			auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, nodeId, browseContext,
														  continuationPoint, referenceDescriptions);
			
			if (uaResult.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			std::list<ModelOpcUa::BrowseResult_t> browseResult;

			if (referenceDescriptions.length() == 0)
			{	
				return OpcUa_ExpandedNodeId();
			}

			if (referenceDescriptions.length() > 1)
			{
				LOG(ERROR) << "Found multiple typeDefinitions for " << nodeId.toXmlString().toUtf8();
				return OpcUa_ExpandedNodeId();
			}

			return referenceDescriptions[0].NodeId;
		}

		UaNodeId OpcUaClient::browseSuperType(const UaNodeId &typeNodeId)
		{
			checkConnection();

			auto referenceTypeUaNodeId = UaNodeId(OpcUaId_HasSubtype);

			UaClientSdk::BrowseContext browseContext;
			browseContext.browseDirection = OpcUa_BrowseDirection_Inverse;
			browseContext.includeSubtype = OpcUa_True;
			browseContext.maxReferencesToReturn = 0;
			browseContext.nodeClassMask = 0; // ALL
			browseContext.referenceTypeId = referenceTypeUaNodeId;
			browseContext.resultMask = OpcUa_BrowseResultMask_None;

			OpcUa_NodeClass nodeClass = readNodeClass(typeNodeId);

			switch (nodeClass)
			{
			case OpcUa_NodeClass_ObjectType:
			case OpcUa_NodeClass_VariableType:
			{
				browseContext.nodeClassMask = nodeClass;
				break;
			}
			default:
				LOG(ERROR) << "Invalid NodeClass " << nodeClass;
				throw Exceptions::UmatiException("Invalid NodeClass");
			}

			UaByteString continuationPoint;
			UaReferenceDescriptions referenceDescriptions;
			auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, typeNodeId, browseContext,
														  continuationPoint, referenceDescriptions);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			std::list<ModelOpcUa::BrowseResult_t> browseResult;

			if (referenceDescriptions.length() == 0)
			{
				return UaNodeId();
			}

			if (referenceDescriptions.length() > 1)
			{
				LOG(ERROR) << "Found multiple superTypes for " << typeNodeId.toXmlString().toUtf8();
				return UaNodeId();
			}

			return UaNodeId(UaExpandedNodeId(referenceDescriptions[0].NodeId).nodeId());
		}

		bool
		OpcUaClient::isSameOrSubtype(const UaNodeId &expectedType, const UaNodeId &checkType, std::size_t maxDepth)
		{
			if (checkType.isNull())
			{
				return false;
			}

			if (expectedType == checkType)
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
			m_opcUaWrapper->SessionUpdateNamespaceTable();

			m_uriToIndexCache.clear();
			m_indexToUriCache.clear();
		}

		void OpcUaClient::updateNamespaceCache()
		{
			UaStringArray uaNamespaces = m_opcUaWrapper->SessionGetNamespaceTable();

			initializeNamespaceCache();

			fillNamespaceCache(uaNamespaces);
		}

		void OpcUaClient::fillNamespaceCache(const UaStringArray &uaNamespaces)
		{
			for (std::size_t i = 0; i < uaNamespaces.length(); ++i)
			{
				auto uaNamespaceUri = uaNamespaces[i];
				auto uaNamespaceUriAsUaString = UaString(uaNamespaceUri);
				auto uaNamespaceUriUtf8 = uaNamespaceUriAsUaString.toUtf8();
				std::string namespaceURI(uaNamespaceUriUtf8);
				m_uriToIndexCache[namespaceURI] = static_cast<uint16_t>(i);
				m_indexToUriCache[static_cast<uint16_t>(i)] = namespaceURI;
				LOG(INFO) << "index: " << std::to_string(i) << ", namespaceURI: " << namespaceURI;
			}
		}

		ModelOpcUa::ModellingRule_t OpcUaClient::browseModellingRule(const UaNodeId &uaNodeId)
		{
			UaByteString continuationPoint;
			UaReferenceDescriptions referenceDescriptions;

			/// begin browse modelling rule
			UaClientSdk::BrowseContext browseContext2 = getUaBrowseContext(prepareObjectAndVariableTypeBrowseContext());
			browseContext2.referenceTypeId = UaNodeId(OpcUaId_HasModellingRule);
			ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::Optional;

			auto uaResult2 = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, uaNodeId,
														   browseContext2,
														   continuationPoint, referenceDescriptions);
			if (uaResult2.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult2.toString().toUtf8() << "for nodeId"
						   << uaNodeId.toFullString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult2);
			}
			for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++)
			{
				auto refDescr = referenceDescriptions[i];
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

		void OpcUaClient::connectionStatusChanged(OpcUa_UInt32 /*clientConnectionId*/,
												  UaClientSdk::UaClient::ServerStatus serverStatus)
		{
			switch (serverStatus)
			{
			case UaClientSdk::UaClient::Disconnected:
				LOG(ERROR) << "Disconnected." << std::endl;
				m_isConnected = false;
				break;
			case UaClientSdk::UaClient::Connected:
				LOG(ERROR) << "Connected." << std::endl;
				m_isConnected = true;
				on_connected();
				break;
			case UaClientSdk::UaClient::ConnectionWarningWatchdogTimeout:
				LOG(ERROR) << "ConnectionWarningWatchdogTimeout." << std::endl;
				break;
			case UaClientSdk::UaClient::ConnectionErrorApiReconnect:
				LOG(ERROR) << "ConnectionErrorApiReconnect." << std::endl;
				break;
			case UaClientSdk::UaClient::ServerShutdown:
				LOG(ERROR) << "ServerShutdown." << std::endl;
				break;
			case UaClientSdk::UaClient::NewSessionCreated:
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
				UaPlatformLayer::cleanup();
			}
		}

		bool OpcUaClient::disconnect()
		{
			if (m_pSession)
			{
				//Subscr.deleteSubscription(m_pSession);
				UaClientSdk::ServiceSettings servsettings;
				return m_opcUaWrapper->SessionDisconnect(servsettings, OpcUa_True).isGood() != OpcUa_False;
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
			UaClientSdk::BrowseContext uaBrowseContext = getUaBrowseContext(browseContext);
			return BrowseWithContextAndFilter(startNode, uaBrowseContext);
		}

		std::list<ModelOpcUa::BrowseResult_t> OpcUaClient::BrowseWithResultTypeFilter(
			ModelOpcUa::NodeId_t startNode,
			BrowseContext_t browseContext,
			ModelOpcUa::NodeId_t typeDefinition)
		{
			UaClientSdk::BrowseContext uaBrowseContext = getUaBrowseContext(browseContext);
			auto typeDefinitionUaNodeId = Converter::ModelNodeIdToUaNodeId(
											  typeDefinition,
											  m_uriToIndexCache)
											  .getNodeId();

			uaBrowseContext.nodeClassMask = nodeClassFromNodeId(typeDefinitionUaNodeId);
			auto filter = [&](const OpcUa_ReferenceDescription &ref) {
				auto browseTypeNodeId = UaNodeId(UaExpandedNodeId(ref.TypeDefinition).nodeId());
				return isSameOrSubtype(typeDefinitionUaNodeId, browseTypeNodeId);
			};
			return BrowseWithContextAndFilter(startNode, uaBrowseContext, filter);
		}

		OpcUa_NodeClass OpcUaClient::nodeClassFromNodeId(const UaNodeId &typeDefinitionUaNodeId)
		{
			OpcUa_NodeClass nodeClass = readNodeClass(typeDefinitionUaNodeId);

			switch (nodeClass)
			{
			case OpcUa_NodeClass_ObjectType:
			{
				return OpcUa_NodeClass_Object;
				break;
			}
			case OpcUa_NodeClass_VariableType:
			{
				return OpcUa_NodeClass_Variable;
				break;
			}
			default:
				LOG(ERROR) << "Invalid NodeClass " << nodeClass
						   << " expect object or variable type for node "
						   << typeDefinitionUaNodeId.toFullString().toUtf8();
				throw Exceptions::UmatiException("Invalid NodeClass");
			}
		}

		std::list<ModelOpcUa::BrowseResult_t> OpcUaClient::BrowseWithContextAndFilter(
			const ModelOpcUa::NodeId_t &startNode,
			UaClientSdk::BrowseContext &browseContext,
			std::function<bool(const OpcUa_ReferenceDescription &)> filter)
		{
			checkConnection();
			auto startUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache).getNodeId();

			UaByteString continuationPoint;
			UaReferenceDescriptions referenceDescriptions;
			auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, startUaNodeId, browseContext,
														  continuationPoint, referenceDescriptions);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8() << ", with startUaNodeId "
						   << startUaNodeId.toFullString().toUtf8()
						   << " and ref id " << browseContext.referenceTypeId.toFullString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			std::list<ModelOpcUa::BrowseResult_t> browseResult;
			ReferenceDescriptionsToBrowseResults(referenceDescriptions, browseResult, filter);
			handleContinuationPoint(continuationPoint);

			return browseResult;
		}

		void OpcUaClient::ReferenceDescriptionsToBrowseResults(
			const UaReferenceDescriptions &referenceDescriptions,
			std::list<ModelOpcUa::BrowseResult_t> &browseResult,
			std::function<bool(const OpcUa_ReferenceDescription &)> filter)
		{
			auto refs = referenceDescriptions;
			for (OpcUa_UInt32 i = 0; i < refs.length(); i++)
			{
				if (!filter(refs[i]))
				{
					continue;
				}
				auto entry = ReferenceDescriptionToBrowseResult(refs[i]);
				browseResult.push_back(entry);
			}
		}

		void OpcUaClient::handleContinuationPoint(const UaByteString & /*continuationPoint*/)
		{
			// LOG(DEBUG) << "Handling continuation point not yet implemented";
		}

		ModelOpcUa::BrowseResult_t
		OpcUaClient::ReferenceDescriptionToBrowseResult(const OpcUa_ReferenceDescription &referenceDescription)
		{
			ModelOpcUa::BrowseResult_t entry;
			auto browseTypeUaNodeId = UaNodeId(UaExpandedNodeId(referenceDescription.TypeDefinition).nodeId());
			entry.NodeClass = Converter::UaNodeClassToModelNodeClass(referenceDescription.NodeClass).getNodeClass();
			entry.TypeDefinition = Converter::UaNodeIdToModelNodeId(browseTypeUaNodeId, m_indexToUriCache).getNodeId();
			entry.NodeId = Converter::UaNodeIdToModelNodeId(
							   UaNodeId(UaExpandedNodeId(referenceDescription.NodeId).nodeId()),
							   m_indexToUriCache)
							   .getNodeId();
			auto referenceTypeUaNodeId = UaNodeId(referenceDescription.ReferenceTypeId);
			auto referenceTypeModelNodeId = Converter::UaNodeIdToModelNodeId(referenceTypeUaNodeId, m_indexToUriCache);
			entry.ReferenceTypeId = referenceTypeModelNodeId.getNodeId();
			entry.BrowseName = Converter::UaQualifiedNameToModelQualifiedName(referenceDescription.BrowseName,
																			  m_indexToUriCache)
								   .getQualifiedName();

			return entry;
		}

		UaClientSdk::BrowseContext OpcUaClient::prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId)
		{
			auto referenceTypeUaNodeId = Converter::ModelNodeIdToUaNodeId(std::move(referenceTypeId),
																		  m_uriToIndexCache)
											 .getNodeId();
			UaClientSdk::BrowseContext browseContext;
			browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
			browseContext.includeSubtype = OpcUa_True;
			browseContext.maxReferencesToReturn = 0;
			browseContext.nodeClassMask = 0; // ALL
			if (nullptr != referenceTypeUaNodeId)
			{
				browseContext.referenceTypeId = referenceTypeUaNodeId;
			}
			browseContext.resultMask =
				OpcUa_BrowseResultMask_BrowseName + OpcUa_BrowseResultMask_TypeDefinition + OpcUa_BrowseResultMask_NodeClass + OpcUa_BrowseResultMask_ReferenceTypeId;
			return browseContext;
		}

		UaClientSdk::BrowseContext OpcUaClient::getUaBrowseContext(const IDashboardDataClient::BrowseContext_t &browseContext)
		{
			UaClientSdk::BrowseContext ret;
			ret.browseDirection = (decltype(ret.browseDirection))browseContext.browseDirection;
			ret.includeSubtype = browseContext.includeSubtypes ? OpcUa_True : OpcUa_False;
			ret.maxReferencesToReturn = 0;
			ret.nodeClassMask = (decltype(ret.nodeClassMask))browseContext.nodeClassMask;
			ret.referenceTypeId = Converter::ModelNodeIdToUaNodeId(browseContext.referenceTypeId, m_uriToIndexCache).getNodeId();
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

			auto startUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache).getNodeId();
			auto uaBrowseName = Converter::ModelQualifiedNameToUaQualifiedName(browseName,
																			   m_uriToIndexCache)
									.getQualifiedName();
			// LOG(INFO) << "translateBrowsePathToNodeId: start from " << startUaNodeId.toString().toUtf8() << " and search " << uaBrowseName.toString().toUtf8();

			UaRelativePathElements uaBrowsePathElements;
			uaBrowsePathElements.create(1);
			uaBrowsePathElements[0].IncludeSubtypes = OpcUa_True;
			uaBrowsePathElements[0].IsInverse = OpcUa_False;
			uaBrowsePathElements[0].ReferenceTypeId.Identifier.Numeric = OpcUaId_HierarchicalReferences;
			uaBrowseName.copyTo(&uaBrowsePathElements[0].TargetName);

			UaBrowsePaths uaBrowsePaths;
			uaBrowsePaths.create(1);
			uaBrowsePaths[0].RelativePath.NoOfElements = uaBrowsePathElements.length();
			uaBrowsePaths[0].RelativePath.Elements = uaBrowsePathElements.detach();
			startUaNodeId.copyTo(&uaBrowsePaths[0].StartingNode);

			UaBrowsePathResults uaBrowsePathResults;
			UaDiagnosticInfos uaDiagnosticInfos;

			auto uaResult = m_opcUaWrapper->SessionTranslateBrowsePathsToNodeIds(
				m_defaultServiceSettings,
				uaBrowsePaths,
				uaBrowsePathResults,
				uaDiagnosticInfos);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "TranslateBrowsePathToNodeId failed for node: '" << static_cast<std::string>(startNode)
						   << "' with " << uaResult.toString().toUtf8() << "(BrowsePath: "
						   << static_cast<std::string>(browseName) << ")";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (uaBrowsePathResults.length() != 1)
			{
				LOG(ERROR) << "Expect 1 browseResult, got " << uaBrowsePathResults.length() << " for node: '"
						   << static_cast<std::string>(startNode)
						   << "' with " << uaResult.toString().toUtf8() << "(BrowsePath: "
						   << static_cast<std::string>(browseName) << ")";
				throw Exceptions::UmatiException("BrowseResult length mismatch.");
			}

			UaStatusCode uaResultElement(uaBrowsePathResults[0].StatusCode);
			if (uaResultElement.isBad())
			{
				std::stringstream ss;
				ss << "Element returned bad status code: " << uaResultElement.toString().toUtf8()
						   << " for node: '"
						   << static_cast<std::string>(startNode) << "' with " << uaResult.toString().toUtf8()
						   << " (BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement, ss.str());
			}

			if (uaBrowsePathResults[0].NoOfTargets != 1)
			{
				LOG(WARNING) << "Continuing with index 0 - expected one target, got "
							 << uaBrowsePathResults[0].NoOfTargets
							 << " for node: '" << static_cast<std::string>(startNode) << "' with "
							 << uaResult.toString().toUtf8() << "(BrowsePath: " << static_cast<std::string>(browseName)
							 << ")";
				for (int target_id = 0; target_id < uaBrowsePathResults[0].NoOfTargets; target_id++)
				{
					try
					{
						UaNodeId _targetNodeId(UaExpandedNodeId(uaBrowsePathResults[0].Targets[0].TargetId).nodeId());
						auto nodeId = Converter::UaNodeIdToModelNodeId(_targetNodeId, m_indexToUriCache).getNodeId();
						LOG(WARNING) << "Target " << target_id << " | id: " << nodeId.Uri << ";" << nodeId.Id;
					}
					catch (std::exception &ex)
					{
						LOG(ERROR) << "error  getting nodeId " << ex.what();
					}
				}
			}

			UaNodeId targetNodeId(UaExpandedNodeId(uaBrowsePathResults[0].Targets[0].TargetId).nodeId());

			return Converter::UaNodeIdToModelNodeId(targetNodeId, m_indexToUriCache).getNodeId();
		}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>
		OpcUaClient::Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback)
		{
			return m_opcUaWrapper->SubscriptionSubscribe(nodeId, callback);
		}

		std::vector<nlohmann::json> OpcUaClient::ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds)
		{
			std::vector<nlohmann::json> ret;
			UaDataValues readValues = readValues2(modelNodeIds);

			for (uint i = 0; i < readValues.length(); ++i)
			{
				auto value = readValues[i];
				auto valu = Converter::UaDataValueToJsonValue(value, false);
				auto val = valu.getValue();
				ret.push_back(val);
			}
			return ret;
		}

		UaDataValues OpcUaClient::readValues2(const std::list<ModelOpcUa::NodeId_t> &modelNodeIds)
		{
			UaStatus uaStatus;
			UaReadValueIds readValueIds;
			readValueIds.resize(modelNodeIds.size());
			unsigned int i = 0;
			for (const auto &modelNodeId : modelNodeIds)
			{
				UaNodeId nodeId = Converter::ModelNodeIdToUaNodeId(modelNodeId, m_uriToIndexCache).getNodeId();
				nodeId.copyTo(&(readValueIds[i].NodeId));
				readValueIds[i].AttributeId = OpcUa_Attributes_Value;
				++i;
			}

			UaDataValues readValues;
			UaDiagnosticInfos diagnosticInfos;

			uaStatus = m_opcUaWrapper->SessionRead(
				m_defaultServiceSettings,
				0,
				OpcUa_TimestampsToReturn::OpcUa_TimestampsToReturn_Neither,
				readValueIds,
				readValues,
				diagnosticInfos);

			if (uaStatus.isNotGood())
			{
				LOG(ERROR) << "Received non good status for read: " << uaStatus.toString().toUtf8();
				std::stringstream ss;
				ss << "Received non good status  for read: " << uaStatus.toString().toUtf8();
				throw Exceptions::OpcUaException(ss.str());
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