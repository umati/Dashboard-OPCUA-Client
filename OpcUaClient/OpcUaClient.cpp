#include "OpcUaClient.hpp"
#include "OpcUaClient.hpp"
#include "OpcUaClient.hpp"
#include "OpcUaClient.hpp"
#include "OpcUaClient.hpp"

#include "OpcUaClient.hpp"
#include <uasession.h>

#include <iostream>
#include "SetupSecurity.hpp"
#include <easylogging++.h>

#include <list>

#include "uadiscovery.h"

#include "uaplatformlayer.h"
#include "OpcUaClient.hpp"

#include <Exceptions/ClientNotConnected.hpp>
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Converter/ModelQualifiedNameToUaQualifiedName.hpp"
#include "Converter/UaQualifiedNameToModelQualifiedName.hpp"
#include "Converter/UaNodeClassToModelNodeClass.hpp"

namespace Umati {

	namespace OpcUa {

		int OpcUaClient::PlattformLayerInitialized = 0;

		OpcUaClient::OpcUaClient(std::string serverURI)
			: m_serverUri(serverURI), m_subscr(m_uriToIndexCache, m_indexToUriCache)
		{
			m_defaultServiceSettings.callTimeout = 1000;


			if (++PlattformLayerInitialized == 1)
			{
				UaPlatformLayer::init();
			}

			m_tryConnecting = true;

			// Try connecting at least once
			this->connect();
			m_connectThread = std::make_shared<std::thread>([this]() {this->threadConnectExecution(); });
		}

		bool OpcUaClient::connect()
		{

			//UaString sURL(serverURI.c_str());
			UaString sURL(m_serverUri.c_str());
			UaStatus result;
			UaClientSdk::SessionConnectInfo sessionConnectInfo;
			sessionConnectInfo.sApplicationName = "KonI4.0 OPC UA Data Client";
			sessionConnectInfo.sApplicationUri = "KonI40OpcUaClient";
			sessionConnectInfo.sProductUri = "KonI40OpcUaClient_Product";
			sessionConnectInfo.sSessionName = "DefaultSession";

			UaClientSdk::SessionSecurityInfo sessionSecurityInfo;
			UaClientSdk::ServiceSettings serviceSettings;
			UaClientSdk::UaDiscovery discovery;
			SetupSecurity::setupSecurity(&sessionSecurityInfo);
			//UaApplicationDescriptions applicationDescriptions;
			//result = discovery.findServers(serviceSettings, sURL, sessionSecurityInfo, applicationDescriptions);

			UaEndpointDescriptions endpointDescriptions;
			result = discovery.getEndpoints(serviceSettings, sURL, sessionSecurityInfo, endpointDescriptions);
			if (result.isBad())
			{
				LOG(ERROR) << "could not get Endpoints.(" << result.toString().toUtf8() << ")" << std::endl;
				return false;
			}

			struct {
				UaString url;
				UaByteString serverCetificate;
				UaString securityPolicy;
				OpcUa_UInt32 securityMode;
			} desiredEnpoint;

			/// \todo
			auto desiredSecurity = OpcUa_MessageSecurityMode_None;

			/// \todo select endpoint dependent on the desired authentification (Anonymous, UserPassword/Cert)
			for (OpcUa_UInt32 iEndpoint = 0; iEndpoint < endpointDescriptions.length(); iEndpoint++)
			{

				if (endpointDescriptions[iEndpoint].SecurityMode != desiredSecurity)
				{
					continue;
				}
				desiredEnpoint.url = UaString(endpointDescriptions[iEndpoint].EndpointUrl);
				LOG(INFO) << "desiredEnpoint.url: " << desiredEnpoint.url.toUtf8() << std::endl;
				sessionSecurityInfo.serverCertificate = endpointDescriptions[iEndpoint].ServerCertificate;
				sessionSecurityInfo.sSecurityPolicy = endpointDescriptions[iEndpoint].SecurityPolicyUri;
				sessionSecurityInfo.messageSecurityMode = static_cast<OpcUa_MessageSecurityMode>(endpointDescriptions[iEndpoint].SecurityMode);
				break;
			}

			if (desiredEnpoint.url.isEmpty())
			{
				LOG(ERROR) << "Could not find endpoint without encryption." << std::endl;
				return false;
			}



			///\todo handle security
			sessionSecurityInfo.doServerCertificateVerify = OpcUa_False;

			m_pSession.reset(new UaClientSdk::UaSession());
			result = m_pSession->connect(sURL, sessionConnectInfo, sessionSecurityInfo, this);

			if (!result.isGood())
			{
				LOG(ERROR) << "Connecting failed in OPC UA Data Client: " << result.toString().toUtf8() << std::endl;
				return false;
			}

			updateNamespaceCache();
			m_subscr.createSubscription(m_pSession);
			return true;
		}

		OpcUa_NodeClass OpcUaClient::readNodeClass(UaNodeId nodeId)
		{
			checkConnection();

			UaReadValueIds readValueIds;
			readValueIds.create(1);
			nodeId.copyTo(&readValueIds[0].NodeId);
			readValueIds[0].AttributeId = OpcUa_Attributes_NodeClass;

			UaDataValues readResult;

			UaDiagnosticInfos diagInfo;

			auto uaResult = m_pSession->read(
				m_defaultServiceSettings,
				100.0,
				OpcUa_TimestampsToReturn_Neither,
				readValueIds,
				readResult,
				diagInfo
			);

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
				LOG(ERROR) << "Bad value status code failed for node: '" << nodeId.toXmlString().toUtf8()
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

			return  static_cast<OpcUa_NodeClass>(nodeClass);
		}

		void OpcUaClient::checkConnection()
		{
			if (!this->m_isConnected || !m_pSession->isConnected())
			{
				throw Exceptions::ClientNotConnected("Need connected client.");
			}
		}

		bool OpcUaClient::isSameOrSubtype(UaNodeId expectedType, UaNodeId checkType)
		{
			///\TODO check subtypes
			return expectedType == checkType;
		}

		void OpcUaClient::updateNamespaceCache()
		{
			///\TODO replace by subcription to ns0;i=2255 [Server_NamespaceArray]
			m_pSession->updateNamespaceTable();
			UaStringArray uaNamespaces = m_pSession->getNamespaceTable();
			for (std::size_t i = 0; i < uaNamespaces.length(); ++i)
			{
				std::string namespaceURI(UaString(uaNamespaces[i]).toUtf8());
				m_uriToIndexCache[namespaceURI] = i;
				m_indexToUriCache[i] = namespaceURI;
			}
		}

		void OpcUaClient::connectionStatusChanged(OpcUa_UInt32 clientConnectionId, UaClientSdk::UaClient::ServerStatus serverStatus)
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

			if (--PlattformLayerInitialized == 0)
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
				return m_pSession->disconnect(servsettings, OpcUa_True).isGood() != OpcUa_False;
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

		std::list < Umati::Dashboard::IDashboardDataClient::BrowseResult_t > OpcUaClient::Browse(
			ModelOpcUa::NodeId_t startNode,
			ModelOpcUa::NodeId_t referenceTypeId,
			ModelOpcUa::NodeId_t typeDefinition)
		{
			checkConnection();

			auto startUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache).getNodeId();
			auto referenceTypeUaNodeId = Converter::ModelNodeIdToUaNodeId(referenceTypeId, m_uriToIndexCache).getNodeId();
			auto typeDefinitionUaNodeId = Converter::ModelNodeIdToUaNodeId(typeDefinition, m_uriToIndexCache).getNodeId();


			UaClientSdk::BrowseContext browseContext;
			browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
			browseContext.includeSubtype = OpcUa_True;
			browseContext.maxReferencesToReturn = 0;
			browseContext.nodeClassMask = 0; // ALL
			browseContext.referenceTypeId = referenceTypeUaNodeId;
			browseContext.resultMask =
				OpcUa_BrowseResultMask_BrowseName |
				OpcUa_BrowseResultMask_TypeDefinition |
				OpcUa_BrowseResultMask_NodeClass |
				OpcUa_BrowseResultMask_ReferenceTypeId;


			OpcUa_NodeClass nodeClass = readNodeClass(typeDefinitionUaNodeId);

			switch (nodeClass)
			{
			case OpcUa_NodeClass_ObjectType:
			{
				browseContext.nodeClassMask = OpcUa_NodeClass_Object;
				break;
			}
			case OpcUa_NodeClass_VariableType:
			{
				browseContext.nodeClassMask = OpcUa_NodeClass_Variable;
				break;
			}
			default:
				LOG(ERROR) << "Invalid NodeClass " << nodeClass;
				throw Exceptions::UmatiException("Invalid NodeClass");
			}

			UaByteString continuationPoint;
			UaReferenceDescriptions referenceDescriptions;
			auto uaResult = m_pSession->browse(m_defaultServiceSettings, startUaNodeId, browseContext, continuationPoint, referenceDescriptions);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			std::list < IDashboardDataClient::BrowseResult_t > browseResult;


			for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++)
			{
				auto browseTypeNodeId = UaNodeId(UaExpandedNodeId(referenceDescriptions[i].TypeDefinition).nodeId());
				if (!isSameOrSubtype(typeDefinitionUaNodeId, browseTypeNodeId))
				{
					continue;
				}

				IDashboardDataClient::BrowseResult_t entry;
				entry.NodeClass = Converter::UaNodeClassToModelNodeClass(referenceDescriptions[i].NodeClass).getNodeClass();
				entry.TypeDefinition = Converter::UaNodeIdToModelNodeId(browseTypeNodeId, m_indexToUriCache).getNodeId();
				entry.NodeId = Converter::UaNodeIdToModelNodeId(
					UaNodeId(UaExpandedNodeId(referenceDescriptions[i].NodeId).nodeId()),
					m_indexToUriCache).getNodeId();
				entry.ReferenceTypeId = Converter::UaNodeIdToModelNodeId(
					UaNodeId(referenceDescriptions[i].ReferenceTypeId),
					m_indexToUriCache).getNodeId();
				entry.BrowseName = Converter::UaQualifiedNameToModelQualifiedName(referenceDescriptions[i].BrowseName, m_indexToUriCache).getQualifiedName();

				browseResult.push_back(entry);
			}

			/// \todo handle continuation point


			return browseResult;
		}

		ModelOpcUa::NodeId_t OpcUaClient::TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode, ModelOpcUa::QualifiedName_t browseName)
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
			auto uaBrowseName = Converter::ModelQualifiedNameToUaQualifiedName(browseName, m_uriToIndexCache).getQualifiedName();

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

			auto uaResult = m_pSession->translateBrowsePathsToNodeIds(
				m_defaultServiceSettings,
				uaBrowsePaths,
				uaBrowsePathResults,
				uaDiagnosticInfos
			);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "TranslateBrowsePathToNodeId failed for node: '" << static_cast<std::string>(startNode)
					<< "' with " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (uaBrowsePathResults.length() != 1)
			{
				LOG(ERROR) << "Expect 1 browseResult, got " << uaBrowsePathResults.length();
				throw Exceptions::UmatiException("BrowseResult length mismatch.");
			}

			UaStatusCode uaResultElement(uaBrowsePathResults[0].StatusCode);
			if (uaResultElement.isBad())
			{
				LOG(ERROR) << "Element returned bad status code: " << uaResultElement.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement);
			}

			if (uaBrowsePathResults[0].NoOfTargets != 1)
			{
				LOG(ERROR) << "Expect 1 traget, got " << uaBrowsePathResults[0].NoOfTargets;
				throw Exceptions::UmatiException("Number of targets mismatch.");
			}

			UaNodeId targetNodeId(UaExpandedNodeId(uaBrowsePathResults[0].Targets[0].TargetId).nodeId());

			return Converter::UaNodeIdToModelNodeId(targetNodeId, m_indexToUriCache).getNodeId();
		}

		void OpcUaClient::UnsubscribeAll()
		{
			m_subscr.UnsubscribeAll();
		}

		void OpcUaClient::Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback)
		{
			m_subscr.Subscribe(nodeId, callback);
		}

	}

}