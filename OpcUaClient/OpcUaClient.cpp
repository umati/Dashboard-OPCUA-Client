
#include "OpcUaClient.hpp"
#include <uasession.h>

#include <iostream>
#include "SetupSecurity.hpp"
#include <easylogging++.h>

#include <list>

#include "uadiscovery.h"

#include "uaplatformlayer.h"
#include "OpcUaClient.hpp"

#include "Exceptions/ClientNotConnected.hpp"
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Converter/ModelQualifiedNameToUaQualifiedName.hpp"

namespace Umati {

	namespace OpcUa {

		int OpcUaClient::PlattformLayerInitialized = 0;

		OpcUaClient::OpcUaClient(std::string serverURI)
			: m_serverUri(serverURI)
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
			//Subscr.setUriToIndexCache(m_uriToIndexCache);
			return true;
			//return Subscr.createSubscription(m_pSession);
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

		std::list < IDashboardClient::BrowseResult_t > OpcUaClient::Browse(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId, ModelOpcUa::NodeId_t typeDefinition)
		{
			return std::list<IDashboardClient::BrowseResult_t>();
		}

		ModelOpcUa::NodeId_t OpcUaClient::TranslateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode, ModelOpcUa::QualifiedName_t browseName)
		{
			if (!this->m_isConnected || !m_pSession->isConnected())
			{
				throw Exceptions::ClientNotConnected("TranslateBrowsePathToNodeId needs connected client.");
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

	}
}