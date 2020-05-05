//
// Created by Dominik on 05.05.2020.
//

#ifndef DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP
#define DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP
#include <uadiscovery.h>
#include <uasession.h>
#include <easylogging++.h>

#include <utility>
#include "Subscription.hpp"

namespace Umati {
    namespace OpcUa {

        class OpcUaSubscriptionInterface {
        public:
            virtual UaStatus SessionCreateSubscription(
                    std::shared_ptr<UaClientSdk::UaSession>& rSession,
                    UaClientSdk::ServiceSettings&        serviceSettings,
                    UaClientSdk::UaSubscriptionCallback* pUaSubscriptionCallback,
                    OpcUa_UInt32            clientSubscriptionHandle,
                    UaClientSdk::SubscriptionSettings&   subscriptionSettings,
                    OpcUa_Boolean           publishingEnabled,
                    UaClientSdk::UaSubscription**        ppUaSubscription
            ) = 0;
            virtual UaStatus SessionDeleteSubscription(
                    std::shared_ptr<UaClientSdk::UaSession>& rSession,
                    UaClientSdk::ServiceSettings& serviceSettings,
                    UaClientSdk::UaSubscription** ppUaSubscription,
                    OpcUa_UInt32 waitTimeForTransactionCompletion = 100) = 0;
        };

        class OpcUaSubscriptionWrapper : public OpcUaSubscriptionInterface {
        public:
            UaStatus SessionCreateSubscription(
                    std::shared_ptr<UaClientSdk::UaSession>& rSession,
                    UaClientSdk::ServiceSettings&        serviceSettings,
                    UaClientSdk::UaSubscriptionCallback* pUaSubscriptionCallback,
                    OpcUa_UInt32            clientSubscriptionHandle,
                    UaClientSdk::SubscriptionSettings&   subscriptionSettings,
                    OpcUa_Boolean           publishingEnabled,
                    UaClientSdk::UaSubscription**        ppUaSubscription) override {
                return rSession->createSubscription(serviceSettings, pUaSubscriptionCallback, clientSubscriptionHandle, subscriptionSettings, publishingEnabled, ppUaSubscription);
            }

            UaStatus SessionDeleteSubscription(
                    std::shared_ptr<UaClientSdk::UaSession>& rSession,
                    UaClientSdk::ServiceSettings& serviceSettings,
                    UaClientSdk::UaSubscription** ppUaSubscription,
                    OpcUa_UInt32 waitTimeForTransactionCompletion = 100) override {
                return rSession->deleteSubscription(serviceSettings, ppUaSubscription, waitTimeForTransactionCompletion);
            }
        };
    }
}
#endif //DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP
