#include "OpcUaClient.hpp"

#include <iostream>
#include "SetupSecurity.hpp"
#include <easylogging++.h>

#include <list>


#include "uaplatformlayer.h"

#include <Exceptions/ClientNotConnected.hpp>
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Converter/ModelQualifiedNameToUaQualifiedName.hpp"
#include "Converter/UaQualifiedNameToModelQualifiedName.hpp"
#include "Converter/UaNodeClassToModelNodeClass.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"


namespace Umati {

	namespace OpcUa {

		int OpcUaClient::PlatformLayerInitialized = 0;

		OpcUaClient::OpcUaClient(std::string serverURI, std::string Username, std::string Password, std::uint8_t security, std::vector<std::string> expectedObjectTypeNamespaces, std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper)
			: m_serverUri(serverURI), m_username(Username), m_password(Password), m_expectedObjectTypeNamespaces(expectedObjectTypeNamespaces), m_security(static_cast<OpcUa_MessageSecurityMode>(security)), m_subscr(m_uriToIndexCache, m_indexToUriCache)
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
			m_connectThread = std::make_shared<std::thread>([this]() {this->threadConnectExecution(); });

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

            result = m_opcUaWrapper->DiscoveryGetEndpoints(serviceSettings, sURL, sessionSecurityInfo,endpointDescriptions);
            if (result.isBad())
			{
                LOG(ERROR) << result.toString().toUtf8();
                return false;
			}

			struct {
				UaString url;
				UaByteString serverCertificate;
				UaString securityPolicy;
				OpcUa_UInt32 securityMode;
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
            //on_connected();

            return true;
		}

        UaClientSdk::SessionConnectInfo &
        OpcUaClient::prepareSessionConnectInfo(UaClientSdk::SessionConnectInfo &sessionConnectInfo) const {
            sessionConnectInfo.sApplicationName = "KonI4.0 OPC UA Data Client";
            sessionConnectInfo.sApplicationUri = "http://dashboard.umati.app/OPCUA_DataClient";
            sessionConnectInfo.sProductUri = "KonI40OpcUaClient_Product";
            sessionConnectInfo.sSessionName = "DefaultSession";
            return sessionConnectInfo;
        }

        void OpcUaClient::on_connected() {
            updateNamespaceCache();
            m_opcUaWrapper->SubscriptionCreateSubscription(m_pSession);
        }

        std::string OpcUaClient::getTypeName(const ModelOpcUa::NodeId_t &nodeId){
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
                LOG(ERROR) << "Bad value status code failed for node: '" << nodeId.toFullString().toUtf8()
                           << "' with " << uaResultElement.toString().toUtf8();
                throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement);
            }

            UaVariant value(readResult[0].Value);
            if (value.type() != OpcUaType_QualifiedName)
            {
                LOG(ERROR) << "Expect Type Int32, got '" << value.type();
                throw Exceptions::UmatiException("Type mismatch");
            }

            return value.toString().toUtf8();
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

			auto uaResult = m_opcUaWrapper->SessionRead(
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
				LOG(ERROR) << "Bad value status code failed for node: '" << nodeId.toFullString().toUtf8()
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
			if (!this->m_isConnected || !m_opcUaWrapper->SessionIsConnected())
			{
				throw Exceptions::ClientNotConnected("Need connected client.");
			}
		}

		UaNodeId OpcUaClient::browseSuperType(UaNodeId typeNodeId)
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
			auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, typeNodeId, browseContext, continuationPoint, referenceDescriptions);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			std::list < ModelOpcUa::BrowseResult_t > browseResult;


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

		bool OpcUaClient::isSameOrSubtype(UaNodeId expectedType, UaNodeId checkType, std::size_t maxDepth)
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

		void OpcUaClient::updateNamespaceCache()
		{
            std::vector<std::string> notFoundObjectTypeNamespaces;
            UaStringArray uaNamespaces = m_opcUaWrapper->SessionGetNamespaceTable();

            initializeUpdateNamespaceCache(notFoundObjectTypeNamespaces);

            for (std::size_t i = 0; i < uaNamespaces.length(); ++i) {
                auto uaNamespace = uaNamespaces[i];
                auto uaNamespaceAsUaString = UaString(uaNamespace);
                auto uaNamespaceUtf8 = uaNamespaceAsUaString.toUtf8();
                std::string namespaceURI(uaNamespaceUtf8);
                m_uriToIndexCache[namespaceURI] = static_cast<uint16_t>(i);
                m_indexToUriCache[static_cast<uint16_t>(i)] = namespaceURI;
                LOG(INFO) << "index: " << std::to_string(i) << ", namespaceURI: " << namespaceURI;
            }


            std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap = std::make_shared<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>>();
            UaClientSdk::BrowseContext browseContext = prepareObjectTypeContext();
            //auto basicObjectTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/MachineTool/","i=1014"}; // todo back to http://opcfoundation.org/UA/; i=58
            auto basicObjectTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/", "i=58"};
            UaNodeId startUaNodeId = Converter::ModelNodeIdToUaNodeId(basicObjectTypeNode, m_uriToIndexCache).getNodeId();

            const ModelOpcUa::BrowseResult_t browseResult{
                ModelOpcUa::NodeClass_t::ObjectType,
                basicObjectTypeNode,
                ModelOpcUa::NodeId_t{"",""}, //TypeDefinition;
                ModelOpcUa::NodeId_t{"",""}, //ReferenceTypeId;
                ModelOpcUa::QualifiedName_t{basicObjectTypeNode.Uri,""} // BrowseName
            };
            auto startType = handleBrowseTypeResult(bidirectionalTypeMap, browseResult, nullptr, ModelOpcUa::ModellingRule_t::Mandatory);
            browseTypes(bidirectionalTypeMap, browseContext, startUaNodeId, startType);


            for (std::size_t i = 0; i < uaNamespaces.length(); ++i) {
                auto uaNamespace = uaNamespaces[i];
                auto uaNamespaceAsUaString = UaString(uaNamespace);
                auto uaNamespaceUtf8 = uaNamespaceAsUaString.toUtf8();
                std::string namespaceURI(uaNamespaceUtf8);
                findObjectTypeNamespaces(notFoundObjectTypeNamespaces, i, namespaceURI, bidirectionalTypeMap);
            }

            for(std::size_t i = 0; i < notFoundObjectTypeNamespaces.size(); ++i){
                LOG(WARNING) << "Unable to find namespace " << notFoundObjectTypeNamespaces[i];
            }

            for(auto mapIterator = m_typeMap->begin(); mapIterator != m_typeMap->end(); mapIterator++) {
                for(auto childIterator = mapIterator->second.SpecifiedChildNodes.begin(); childIterator != mapIterator->second.SpecifiedChildNodes.end(); childIterator++) {
                    std::string childTypeName = getTypeName(childIterator->get()->SpecifiedTypeNodeId);
                    auto childType = m_typeMap->find(childTypeName);
                    if(childType != m_typeMap->end()) {
                        childIterator->operator=(std::make_shared<ModelOpcUa::StructureNode>(childIterator->get(),childType->second.SpecifiedChildNodes));
                    }
                }
            }
            LOG(INFO) << "Updated typemap";
		}

        void OpcUaClient::findObjectTypeNamespaces(std::vector<std::string> &notFoundObjectTypeNamespaces, size_t i,
                                                   const std::string &namespaceURI, std::shared_ptr<std::map <std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap) {
            auto it = find (notFoundObjectTypeNamespaces.begin(), notFoundObjectTypeNamespaces.end(), namespaceURI);
            if (it != notFoundObjectTypeNamespaces.end()) {
                NamespaceInformation_t information;

                std::vector<std::string> resultContainer;
                split(namespaceURI, resultContainer, '/');

                information.Namespace = resultContainer.back();
                information.NamespaceUri = namespaceURI;

                std::stringstream typeName;
                std::stringstream identificationTypeName;
                typeName << information.Namespace << "Type";
                identificationTypeName << information.Namespace << "IdentificationType";

                information.NamespaceType = typeName.str();
                information.NamespaceIdentificationType = identificationTypeName.str();

                m_availableObjectTypeNamespaces[static_cast<uint16_t>(i)] = information;
                notFoundObjectTypeNamespaces.erase(it);
                LOG(INFO) << "Expected object type namespace " << namespaceURI << " found at index " << std::to_string(i);
                createTypeMap(bidirectionalTypeMap, m_typeMap, i);
                LOG(INFO) << "Finished creatingTypeMap for " << namespaceURI;
            }
        }

        void OpcUaClient::browseUnderStartNode(UaNodeId startUaNodeId, UaReferenceDescriptions &referenceDescriptions) {
            browseUnderStartNode(startUaNodeId, referenceDescriptions, prepareObjectTypeContext());
		}

        void OpcUaClient::browseUnderStartNode(UaNodeId startUaNodeId,UaReferenceDescriptions &referenceDescriptions, UaClientSdk::BrowseContext browseContext) {
            UaByteString continuationPoint;
            // References -> nodes referenced to this, e.g. child nodes
            // BrowseName: Readable Name and namespace index
            auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, startUaNodeId, browseContext, continuationPoint, referenceDescriptions);
            if (uaResult.isBad()) {
                LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8() << " for node " << startUaNodeId.toFullString().toUtf8();
                throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
            }
        }

        void OpcUaClient::browseTypes(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap, UaClientSdk::BrowseContext browseContext, UaNodeId startUaNodeId, const std::shared_ptr<ModelOpcUa::StructureBiNode>& parent)  {

            UaReferenceDescriptions referenceDescriptions;
            browseUnderStartNode(startUaNodeId, referenceDescriptions, browseContext);

            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++)
            {
                ModelOpcUa::BrowseResult_t browseResult = ReferenceDescriptionToBrowseResult(referenceDescriptions[i]);
                UaNodeId nextUaNodeId = Converter::ModelNodeIdToUaNodeId(browseResult.NodeId, m_uriToIndexCache).getNodeId();
                ModelOpcUa::ModellingRule_t modellingRule = browseModellingRule(nextUaNodeId);
                // LOG(INFO) << "currently at " << startUaNodeId.toFullString().toUtf8();
                auto current = handleBrowseTypeResult(bidirectionalTypeMap, browseResult, parent, modellingRule);
                browseTypes(bidirectionalTypeMap, browseContext, nextUaNodeId, current);
            }
        }

        ModelOpcUa::ModellingRule_t OpcUaClient::browseModellingRule(UaNodeId uaNodeId) {
            UaByteString continuationPoint;
            UaReferenceDescriptions referenceDescriptions;

            /// begin browse modelling rule
            UaClientSdk::BrowseContext browseContext2 = prepareObjectTypeContext();
            browseContext2.referenceTypeId = UaNodeId(OpcUaId_HasModellingRule);
            ModelOpcUa::ModellingRule_t modellingRule = ModelOpcUa::ModellingRule_t::Optional;

            auto uaResult2 = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, uaNodeId,
                                                           browseContext2,
                                                           continuationPoint, referenceDescriptions);
            if (uaResult2.isBad()) {
                LOG(ERROR) << "Bad return from browse: " << uaResult2.toString().toUtf8() << "for nodeId" << uaNodeId.toFullString().toUtf8();
                throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult2);
            }
            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++) {
                auto refDescr = referenceDescriptions[i];
                ModelOpcUa::BrowseResult_t browseResult = ReferenceDescriptionToBrowseResult(refDescr);
                if (browseResult.BrowseName.Name == "Mandatory") {
                    modellingRule = ModelOpcUa::Mandatory;
                } else if (browseResult.BrowseName.Name == "Optional") {
                    modellingRule = ModelOpcUa::Optional;
                } else if (browseResult.BrowseName.Name == "MandatoryPlaceholder") {
                    modellingRule = ModelOpcUa::MandatoryPlaceholder;
                } else if (browseResult.BrowseName.Name == "OptionalPlaceholder") {
                    modellingRule = ModelOpcUa::OptionalPlaceholder;
                }
            }
            return modellingRule;
		}

        std::shared_ptr<ModelOpcUa::StructureBiNode> OpcUaClient::handleBrowseTypeResult(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap,
                                            const ModelOpcUa::BrowseResult_t &entry,
                                            const std::shared_ptr<ModelOpcUa::StructureBiNode>& parent, ModelOpcUa::ModellingRule_t modellingRule) {
            UaNodeId currentUaNodeId = Converter::ModelNodeIdToUaNodeId(entry.NodeId, m_uriToIndexCache).getNodeId();
            uint16_t currentNamespaceIndex = currentUaNodeId.namespaceIndex();
            auto it = m_availableObjectTypeNamespaces.find(currentNamespaceIndex);
            bool isObjectType = ModelOpcUa::ObjectType == entry.NodeClass;
            ModelOpcUa::StructureBiNode node(entry, std::list<std::shared_ptr<const ModelOpcUa::StructureNode>>(), parent, (uint16_t) currentUaNodeId.namespaceIndex(),modellingRule );
            auto current = std::make_shared<ModelOpcUa::StructureBiNode>(node);

            if (isObjectType) {
                std::string typeName = node.structureNode->SpecifiedBrowseName.Name;
                if(bidirectionalTypeMap->count(typeName) == 0) {
                    current->isType = true;
                    std::pair <std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>> newType(typeName, current);
                    bidirectionalTypeMap->insert(newType);
                    std::pair <std::string, ModelOpcUa::NodeId_t> newNameMapping(typeName, entry.NodeId);
                    m_nameToId->insert(newNameMapping);
                    if((bidirectionalTypeMap->size()%50) == 0){
                        LOG(INFO) << "Current size BiDirectionalTypeMap: " << bidirectionalTypeMap->size();
                    }
                } else {
                    LOG(INFO) << "Found Type " << typeName << " again.";
                }
            }
            if (parent != nullptr) {
                parent->SpecifiedBiChildNodes.emplace_back(current);
            }
            return current;
       }

        UaClientSdk::BrowseContext OpcUaClient::prepareObjectTypeContext() const {
            UaClientSdk::BrowseContext browseContext;
            browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
            browseContext.includeSubtype = OpcUa_True;
            browseContext.maxReferencesToReturn = 0;

            /*
             * - OpcUa_NodeClass_Object        = 1,
             * - OpcUa_NodeClass_Variable      = 2,
             * - OpcUa_NodeClass_Method        = 4,
             * - OpcUa_NodeClass_ObjectType    = 8,
             * - OpcUa_NodeClass_VariableType  = 16,
             * - OpcUa_NodeClass_ReferenceType = 32,
             * - OpcUa_NodeClass_DataType      = 64,
             * - OpcUa_NodeClass_View          = 128
             * */
            browseContext.nodeClassMask =
                    OpcUa_NodeClass_ObjectType |
                    OpcUa_NodeClass_Object |
                    OpcUa_NodeClass_Variable |
                    OpcUa_NodeClass_VariableType
                ;
            browseContext.resultMask = OpcUa_BrowseResultMask_All ;
            return browseContext;
        }


        void OpcUaClient::initializeUpdateNamespaceCache(std::vector<std::string> &notFoundObjectTypeNamespaces) {
            m_opcUaWrapper->SessionUpdateNamespaceTable();

            m_uriToIndexCache.clear();
            m_indexToUriCache.clear();
            m_availableObjectTypeNamespaces.clear();

            for (std::size_t i = 0; i < m_expectedObjectTypeNamespaces.size(); i++) {
                notFoundObjectTypeNamespaces.push_back(m_expectedObjectTypeNamespaces[i]);
            }
        }

        void OpcUaClient::connectionStatusChanged(OpcUa_UInt32 /*clientConnectionId*/, UaClientSdk::UaClient::ServerStatus serverStatus)
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

		std::list < ModelOpcUa::BrowseResult_t > OpcUaClient::Browse(
			ModelOpcUa::NodeId_t startNode,
			ModelOpcUa::NodeId_t referenceTypeId,
			ModelOpcUa::NodeId_t typeDefinition)
		{
            UaClientSdk::BrowseContext browseContext = prepareBrowseContext(referenceTypeId);
            return BrowseWithContext(startNode, referenceTypeId, typeDefinition, browseContext);
        }

        std::list<ModelOpcUa::BrowseResult_t>
        OpcUaClient::BrowseWithContext(const ModelOpcUa::NodeId_t &startNode,
                                       const ModelOpcUa::NodeId_t &referenceTypeId,
                                       const ModelOpcUa::NodeId_t &typeDefinition,
                                       UaClientSdk::BrowseContext &browseContext) {
            checkConnection();
            auto startUaNodeId = Converter::ModelNodeIdToUaNodeId(startNode, m_uriToIndexCache).getNodeId();
            auto referenceTypeUaNodeId = Converter::ModelNodeIdToUaNodeId(referenceTypeId, m_uriToIndexCache).getNodeId();
            auto typeDefinitionUaNodeId = Converter::ModelNodeIdToUaNodeId(typeDefinition, m_uriToIndexCache).getNodeId();


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
                LOG(ERROR) << "Invalid NodeClass " << nodeClass
                    << " expect object or variable type for node " << static_cast<std::string>(typeDefinition);
                throw Exceptions::UmatiException("Invalid NodeClass");
            }

            UaByteString continuationPoint;
            UaReferenceDescriptions referenceDescriptions;
            auto uaResult = m_opcUaWrapper->SessionBrowse(m_defaultServiceSettings, startUaNodeId, browseContext, continuationPoint, referenceDescriptions);

            if (uaResult.isBad())
            {
                LOG(ERROR) << "Bad return from browse: " << uaResult.toString().toUtf8() << ", with startUaNodeId " << startUaNodeId.toFullString().toUtf8()
                << " and ref id " << browseContext.referenceTypeId.toFullString().toUtf8() ;
                throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
            }

            std::__cxx11::list < ModelOpcUa::BrowseResult_t > browseResult;
            ReferenceDescriptionsToBrowseResults(typeDefinitionUaNodeId, referenceDescriptions, browseResult);
            handleContinuationPoint(continuationPoint);

            return browseResult;
        }

        void OpcUaClient::ReferenceDescriptionsToBrowseResults(const UaNodeId &typeDefinitionUaNodeId,
                                                               const UaReferenceDescriptions &referenceDescriptions,
                                                               std::list<ModelOpcUa::BrowseResult_t> &browseResult) {
            for (OpcUa_UInt32 i = 0; i < referenceDescriptions.length(); i++)
            {
                auto browseTypeNodeId = UaNodeId(UaExpandedNodeId(referenceDescriptions[i].TypeDefinition).nodeId());
                if (!isSameOrSubtype(typeDefinitionUaNodeId, browseTypeNodeId))
                {
                    continue;
                }

                auto entry = ReferenceDescriptionToBrowseResult(referenceDescriptions[i]);

                browseResult.push_back(entry);
            }
        }

        void OpcUaClient::handleContinuationPoint(const UaByteString &continuationPoint) const {
		    //todo handle continuation point
            LOG(INFO) << "Handling continuation point not yet implemented";
        }

        ModelOpcUa::BrowseResult_t OpcUaClient::ReferenceDescriptionToBrowseResult(const OpcUa_ReferenceDescription &referenceDescription) {
            ModelOpcUa::BrowseResult_t entry;
            auto browseTypeUaNodeId = UaNodeId(UaExpandedNodeId(referenceDescription.TypeDefinition).nodeId());
            entry.NodeClass = Converter::UaNodeClassToModelNodeClass(referenceDescription.NodeClass).getNodeClass();
            entry.TypeDefinition = Converter::UaNodeIdToModelNodeId(browseTypeUaNodeId,m_indexToUriCache).getNodeId();
            entry.NodeId = Converter::UaNodeIdToModelNodeId(UaNodeId(UaExpandedNodeId(referenceDescription.NodeId).nodeId()),m_indexToUriCache).getNodeId();
            auto referenceTypeUaNodeId = UaNodeId(referenceDescription.ReferenceTypeId);
            auto referenceTypeModelNodeId = Converter::UaNodeIdToModelNodeId(referenceTypeUaNodeId,m_indexToUriCache);
            entry.ReferenceTypeId = referenceTypeModelNodeId.getNodeId();
            entry.BrowseName = Converter::UaQualifiedNameToModelQualifiedName(referenceDescription.BrowseName,m_indexToUriCache).getQualifiedName();

            return entry;
        }

        UaClientSdk::BrowseContext OpcUaClient::prepareBrowseContext(ModelOpcUa::NodeId_t referenceTypeId) {
            auto referenceTypeUaNodeId = Converter::ModelNodeIdToUaNodeId(referenceTypeId, m_uriToIndexCache).getNodeId();
            UaClientSdk::BrowseContext browseContext;
            browseContext.browseDirection = OpcUa_BrowseDirection_Forward;
            browseContext.includeSubtype = OpcUa_True;
            browseContext.maxReferencesToReturn = 0;
            browseContext.nodeClassMask = 0; // ALL
            if (nullptr != referenceTypeUaNodeId) {
                browseContext.referenceTypeId = referenceTypeUaNodeId;
            }
            browseContext.resultMask =
                OpcUa_BrowseResultMask_BrowseName |
                OpcUa_BrowseResultMask_TypeDefinition |
                OpcUa_BrowseResultMask_NodeClass |
                OpcUa_BrowseResultMask_ReferenceTypeId;
            return browseContext;
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
			LOG(INFO) << startUaNodeId.toString().toUtf8() << uaBrowseName.toString().toUtf8();

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
				uaDiagnosticInfos
			);

			if (uaResult.isBad())
			{
				LOG(ERROR) << "TranslateBrowsePathToNodeId failed for node: '" << static_cast<std::string>(startNode)
					<< "' with " << uaResult.toString().toUtf8() << "(BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (uaBrowsePathResults.length() != 1)
			{
				LOG(ERROR) << "Expect 1 browseResult, got " << uaBrowsePathResults.length() << " for node: '" << static_cast<std::string>(startNode)
					<< "' with " << uaResult.toString().toUtf8() << "(BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::UmatiException("BrowseResult length mismatch.");
			}

			UaStatusCode uaResultElement(uaBrowsePathResults[0].StatusCode);
			if (uaResultElement.isBad())
			{
				LOG(ERROR) << "Element returned bad status code: " << uaResultElement.toString().toUtf8() << " for node: '" << static_cast<std::string>(startNode)
					<< "' with " << uaResult.toString().toUtf8() << "(BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultElement);
			}

			if (uaBrowsePathResults[0].NoOfTargets != 1)
			{
				LOG(ERROR) << "Expect 1 target, got " << uaBrowsePathResults[0].NoOfTargets << " for node: '" << static_cast<std::string>(startNode)
					<< "' with " << uaResult.toString().toUtf8() << "(BrowsePath: " << static_cast<std::string>(browseName) << ")";
				throw Exceptions::UmatiException("Number of targets mismatch.");
			}

			UaNodeId targetNodeId(UaExpandedNodeId(uaBrowsePathResults[0].Targets[0].TargetId).nodeId());

			return Converter::UaNodeIdToModelNodeId(targetNodeId, m_indexToUriCache).getNodeId();
		}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> OpcUaClient::Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback)
		{
			return m_opcUaWrapper->SubscriptionSubscribe(nodeId, callback);
		}

		std::vector<nlohmann::json> OpcUaClient::readValues(std::list<ModelOpcUa::NodeId_t> modelNodeIds)
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

        UaDataValues OpcUaClient::readValues2(std::list<ModelOpcUa::NodeId_t> modelNodeIds)
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

        void OpcUaClient::createTypeMap(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap, std::shared_ptr<std::map<std::string, ModelOpcUa::StructureNode>> typeMap, uint16_t namespaceIndex) {
		    for ( auto typeIterator = bidirectionalTypeMap->begin(); typeIterator != bidirectionalTypeMap->end(); typeIterator++ ) {
                if(typeIterator->second->namespaceIndex != namespaceIndex) {
                    continue;
                }
                // go to highest parent and then down the ladder to add / update attributes;
                // create a list of pointers till parent is null
                // go backwards and add / update child nodes till the end
                std::list<std::shared_ptr<ModelOpcUa::StructureBiNode>> bloodline;
                std::shared_ptr<ModelOpcUa::StructureBiNode> currentGeneration = typeIterator->second;
                while(nullptr != currentGeneration) {
                    bloodline.emplace_back(currentGeneration);
                    currentGeneration = currentGeneration->parent;
                }
                std::string typeName = bloodline.front()->structureNode->SpecifiedBrowseName.Name;
                ModelOpcUa::StructureNode node = bloodline.front()->structureNode.operator*();
                std::stringstream bloodlineStringStream;
                for(auto bloodlineIterator = bloodline.end(); bloodlineIterator != bloodline.begin(); ){
                    --bloodlineIterator;
                    auto ancestor = bloodlineIterator.operator*();
                    bloodlineStringStream << "->" << ancestor->structureNode->SpecifiedBrowseName.Name;
                    for(auto childIterator = ancestor->SpecifiedBiChildNodes.begin(); childIterator != ancestor->SpecifiedBiChildNodes.end(); childIterator++) {
                        auto currentChild = childIterator.operator*();
                        if(!currentChild->isType){
                            auto structureNode = currentChild->toStructureNode();

                            auto findIterator = std::find(node.SpecifiedChildNodes.begin(), node.SpecifiedChildNodes.end(), structureNode);

                            bool found = findIterator != node.SpecifiedChildNodes.end();
                            if (!found) {
                                for(auto fIt = node.SpecifiedChildNodes.begin(); fIt !=  node.SpecifiedChildNodes.end();  fIt++) {
                                    if (fIt.operator*()->SpecifiedBrowseName.Name == structureNode->SpecifiedBrowseName.Name &&
                                    fIt.operator*()->SpecifiedBrowseName.Uri == structureNode->SpecifiedBrowseName.Uri
                                    ) {
                                        findIterator = fIt;
                                        found = true;
                                        break;
                                    }
                                }
                            }

                            if (found) {
                                if (findIterator.operator*()->ModellingRule == ModelOpcUa::ModellingRule_t::Optional ||
                                findIterator.operator*()->ModellingRule == ModelOpcUa::ModellingRule_t::OptionalPlaceholder) {
                                    LOG(INFO) << "Changed modellingRule from " << findIterator.operator*()->ModellingRule << " to " << structureNode->ModellingRule;
                                    node.SpecifiedChildNodes.erase(findIterator++);
                                    node.SpecifiedChildNodes.emplace_back(structureNode);
                                }
                            } else {
                                node.SpecifiedChildNodes.emplace_back(structureNode);
                            }
                        }
                    }
                }
                auto shared = std::make_shared<ModelOpcUa::StructureNode>(node);

                LOG(INFO) << std::endl << ModelOpcUa::StructureNode::printJson(shared);
                std::pair <std::string, ModelOpcUa::StructureNode> newType(typeName, node);
                typeMap->insert(newType);
            }
        }

        void OpcUaClient::split(const std::string& inputString, std::vector<std::string>& resultContainer, char delimiter)
        {
            std::size_t current_char_position, previous_char_position = 0;
            current_char_position = inputString.find(delimiter);
            while (current_char_position != std::string::npos) {
                updateResultContainer(inputString, resultContainer, current_char_position, previous_char_position);
                previous_char_position = current_char_position + 1;
                current_char_position = inputString.find(delimiter, previous_char_position);
            }
            updateResultContainer(inputString, resultContainer, current_char_position, previous_char_position);
        }

        void
        OpcUaClient::updateResultContainer(const std::string &inputString, std::vector<std::string> &resultContainer, size_t current_char_position, size_t previous_char_position) const {
            std::string substr = inputString.substr(previous_char_position, current_char_position - previous_char_position);
            if(!substr.empty()){
                resultContainer.push_back(substr);
            }
        }
    }
}