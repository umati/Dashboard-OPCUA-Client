//
// Created by Dominik on 24.04.2020.
//

#ifndef DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP
#define DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP

#include <uadiscovery.h>
#include <uasession.h>
#include <easylogging++.h>

#include <utility>
#include "Subscription.hpp"

namespace Umati {
    namespace OpcUa {
        class OpcUaInterface {

        public:

            virtual UaStatus DiscoveryGetEndpoints(UaClientSdk::ServiceSettings &serviceSettings,
                                                   const UaString &sDiscoveryURL,
                                                   UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                                                   UaEndpointDescriptions &endpointDescriptions) = 0;
            virtual UaStatus DiscoveryFindServers(
                    UaClientSdk::ServiceSettings &serviceSettings,
                    const UaString &sDiscoveryURL,
                    UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                    UaApplicationDescriptions &applicationDescriptions) = 0;
            virtual void GetNewSession(std::shared_ptr<UaClientSdk::UaSession>& m_pSession) = 0;
            virtual UaStatus SessionConnect(const UaString&      sURL,
                                            UaClientSdk::SessionConnectInfo&  sessionConnectInfo,
                                            UaClientSdk::SessionSecurityInfo& sessionSecurityInfo,
                                            UaClientSdk::UaSessionCallback*   pSessionCallback) = 0;
            virtual UaStatus SessionDisconnect(UaClientSdk::ServiceSettings& serviceSettings,
                                           OpcUa_Boolean    bDeleteSubscriptions) = 0;
            virtual void SessionUpdateNamespaceTable() = 0;
            virtual UaStringArray SessionGetNamespaceTable() = 0;
            virtual UaStatus SessionRead(UaClientSdk::ServiceSettings&         serviceSettings,
                                         OpcUa_Double             maxAge,
                                         OpcUa_TimestampsToReturn timeStamps,
                                         const UaReadValueIds&    nodesToRead,
                                         UaDataValues&            values,
                                         UaDiagnosticInfos&       diagnosticInfos) = 0;
            virtual bool SessionIsConnected() = 0;
            virtual UaStatus SessionBrowse(
                    UaClientSdk::ServiceSettings&         serviceSettings,
                    const UaNodeId&          nodeToBrowse,
                    const UaClientSdk::BrowseContext&     browseContext,
                    UaByteString&            continuationPoint,
                    UaReferenceDescriptions& referenceDescriptions) = 0;
            virtual UaStatus SessionTranslateBrowsePathsToNodeIds(
                    UaClientSdk::ServiceSettings&     serviceSettings,
                    const UaBrowsePaths& browsePaths,
                    UaBrowsePathResults& browsePathResults,
                    UaDiagnosticInfos&   diagnosticInfos
            ) = 0;
            virtual void setSubscription(Subscription* p_in_subscr) = 0;
            virtual void SubscriptionCreateSubscription(std::shared_ptr<UaClientSdk::UaSession>& m_pSession) = 0;
            virtual std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> SubscriptionSubscribe(	ModelOpcUa::NodeId_t nodeId, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback) = 0;
        protected:

            Subscription* p_subscr;
        };

        class OpcUaWrapper: public OpcUaInterface {
        public:

            UaStatus DiscoveryGetEndpoints(UaClientSdk::ServiceSettings &serviceSettings,
                                           const UaString &sDiscoveryURL,
                                           UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                                           UaEndpointDescriptions &endpointDescriptions) override {
                UaClientSdk::UaDiscovery discovery;
                return discovery.getEndpoints(serviceSettings, sDiscoveryURL, clientSecurityInfo, endpointDescriptions);
            };

            UaStatus DiscoveryFindServers(
                    UaClientSdk::ServiceSettings &serviceSettings,
                    const UaString &sDiscoveryURL,
                    UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                    UaApplicationDescriptions &applicationDescriptions) override {
                UaClientSdk::UaDiscovery discovery;
                return discovery.findServers(serviceSettings, sDiscoveryURL, clientSecurityInfo,
                                             applicationDescriptions);
            };

            void GetNewSession(std::shared_ptr<UaClientSdk::UaSession>& m_pSession) override {
                m_pSession.reset(new UaClientSdk::UaSession());
                pSession = m_pSession;
            }

            UaStatus SessionConnect(const UaString&      sURL,
                                            UaClientSdk::SessionConnectInfo&  sessionConnectInfo,
                                            UaClientSdk::SessionSecurityInfo& sessionSecurityInfo,
                                            UaClientSdk::UaSessionCallback*   pSessionCallback) override {
                    return pSession->connect(sURL, sessionConnectInfo, sessionSecurityInfo, pSessionCallback);
            }

            UaStatus SessionDisconnect(UaClientSdk::ServiceSettings& serviceSettings,
                                       OpcUa_Boolean    bDeleteSubscriptions) override {
                return pSession->disconnect(serviceSettings, bDeleteSubscriptions);
            }

            UaStatus SessionRead(UaClientSdk::ServiceSettings&         serviceSettings,
                                 OpcUa_Double             maxAge,
                                 OpcUa_TimestampsToReturn timeStamps,
                                 const UaReadValueIds&    nodesToRead,
                                 UaDataValues&            values,
                                 UaDiagnosticInfos&       diagnosticInfos) override {
                return pSession->read(serviceSettings, maxAge, timeStamps, nodesToRead, values, diagnosticInfos);
            }

            bool SessionIsConnected() override {
                return pSession->isConnected();
            }

            void SessionUpdateNamespaceTable() override {
                pSession->updateNamespaceTable();
            }

            UaStringArray SessionGetNamespaceTable() override {
                return pSession->getNamespaceTable();
            }

            UaStatus SessionBrowse(
                    UaClientSdk::ServiceSettings&         serviceSettings,
                    const UaNodeId&          nodeToBrowse,
                    const UaClientSdk::BrowseContext&     browseContext,
                    UaByteString&            continuationPoint,
                    UaReferenceDescriptions& referenceDescriptions) override {
                return pSession->browse(serviceSettings, nodeToBrowse, browseContext, continuationPoint, referenceDescriptions);
            }

            UaStatus SessionTranslateBrowsePathsToNodeIds(
                    UaClientSdk::ServiceSettings&     serviceSettings,
                    const UaBrowsePaths& browsePaths,
                    UaBrowsePathResults& browsePathResults,
                    UaDiagnosticInfos&   diagnosticInfos
            ) override {
                return pSession->translateBrowsePathsToNodeIds(serviceSettings, browsePaths, browsePathResults, diagnosticInfos);
            }

            void setSubscription(Subscription* p_in_subscr) override {p_subscr = p_in_subscr;}

            void SubscriptionCreateSubscription(std::shared_ptr<UaClientSdk::UaSession>& m_pSession) override {
                if (p_subscr == nullptr) {
                    LOG(ERROR) << "Unable to create subscription, pointer is NULL ";
                    exit(SIGTERM);
                }
                p_subscr->createSubscription(m_pSession);
            };

            std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> SubscriptionSubscribe(ModelOpcUa::NodeId_t nodeId,
                    Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback) override {
                if (p_subscr == nullptr) {
                    LOG(ERROR) << "Unable to subscribe, pointer is NULL ";
                    exit(SIGTERM);
                }
                return p_subscr->Subscribe(nodeId, callback);
            }

            std::shared_ptr<UaClientSdk::UaSession> pSession;
        };
    }
}

#endif //DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP
