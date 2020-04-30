//
// Created by Dominik on 24.04.2020.
//

#ifndef DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP
#define DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP

#include <uadiscovery.h>
#include <uasession.h>

namespace Umati {
    namespace OpcUa {
        class OpcUaInterface {

        public:

            virtual UaStatus GetEndpoints(UaClientSdk::ServiceSettings &serviceSettings,
                                          const UaString &sDiscoveryURL,
                                          UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                                          UaEndpointDescriptions &endpointDescriptions) = 0;
            virtual void GetNewSession(std::shared_ptr<UaClientSdk::UaSession>& m_pSession) = 0;
            virtual UaStatus SessionConnect(const UaString&      sURL,
                                            UaClientSdk::SessionConnectInfo&  sessionConnectInfo,
                                            UaClientSdk::SessionSecurityInfo& sessionSecurityInfo,
                                            UaClientSdk::UaSessionCallback*   pSessionCallback) = 0;
            virtual void SessionUpdateNamespaceTable() = 0;
            virtual UaStringArray SessionGetNamespaceTable() = 0;
        };

        class OpcUaWrapper: public OpcUaInterface {
        public:

            UaStatus GetEndpoints(UaClientSdk::ServiceSettings &serviceSettings,
                                          const UaString &sDiscoveryURL,
                                          UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
                                          UaEndpointDescriptions &endpointDescriptions) override {
                UaClientSdk::UaDiscovery discovery;
                return discovery.getEndpoints(serviceSettings, sDiscoveryURL, clientSecurityInfo, endpointDescriptions);
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

            void SessionUpdateNamespaceTable() override {
                pSession->updateNamespaceTable();
            }

            UaStringArray SessionGetNamespaceTable() override {
                return pSession->getNamespaceTable();
            }

            std::shared_ptr<UaClientSdk::UaSession> pSession;
        };
    }
}

#endif //DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP
