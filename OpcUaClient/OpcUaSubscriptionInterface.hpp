//
// Created by Dominik on 05.05.2020.
//

#ifndef DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP
#define DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP

#include <easylogging++.h>

#include <utility>
#include "Subscription.hpp"

namespace Umati {
	namespace OpcUa {

		class OpcUaSubscriptionInterface {

		public:
		typedef void (*UA_Client_DeleteSubscriptionCallback)
   	 	(UA_Client *client, UA_UInt32 subId, void *subContext);

		typedef void (*UA_Client_StatusChangeNotificationCallback)
    	(UA_Client *client, UA_UInt32 subId, void *subContext,
     	UA_StatusChangeNotification *notification);

			virtual UA_CreateSubscriptionResponse SessionCreateSubscription(
					UA_Client *client,
					const UA_CreateSubscriptionRequest request,
					void *subscriptionContext,
   					UA_Client_StatusChangeNotificationCallback statusChangeCallback,
    				UA_Client_DeleteSubscriptionCallback deleteCallback
			) = 0;

			virtual UA_StatusCode SessionDeleteSubscription(	
					UA_Client *client,
    				const UA_Int32 subscriptionId) = 0;
		};

		class OpcUaSubscriptionWrapper : public OpcUaSubscriptionInterface {
		public:
			UA_CreateSubscriptionResponse SessionCreateSubscription(					
				  UA_Client *client,
					const UA_CreateSubscriptionRequest request,
					void *subscriptionContext,
   					UA_Client_StatusChangeNotificationCallback statusChangeCallback,
    				UA_Client_DeleteSubscriptionCallback deleteCallback) override {

				return UA_Client_Subscriptions_create(client,request,subscriptionContext, statusChangeCallback, deleteCallback);
			}

			UA_StatusCode SessionDeleteSubscription(	
						UA_Client *client,
    				const UA_Int32 subscriptionId) override {
						return UA_Client_Subscriptions_deleteSingle(client, subscriptionId);
						
				}
		};
	}
}
#endif //DASHBOARD_OPCUACLIENT_OPCUASUBSCRIPTIONINTERFACE_HPP
