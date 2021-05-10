#pragma once

#include <open62541/client.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <Open62541Cpp/UA_NodeId.hpp>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <atomic>
#include <IDashboardDataClient.hpp>
#include "OpcUaSubscriptionInterface.hpp"

namespace Umati {
	namespace OpcUa  {
		class Subscription {//: public UA_SubscriptionAcknowledgement{
		public:

			Subscription(const std::map<std::string, uint16_t> &m_uriToIndexCache,
						 const std::map<uint16_t, std::string> &m_indexToUriCache);

			void subscriptionStatusChanged(UA_Client *client,UA_Int32 clientSubscriptionHandle, const UA_StatusCode &status);// override;

			void dataChange(UA_Int32 clientSubscriptionHandle, const UA_DataChangeNotification &dataNotifications,
							const UA_DiagnosticInfo &diagnosticInfos);// override;

			void newEvents(UA_Int32 clientSubscriptionHandle, UA_EventFieldList &eventFieldList); // override;

			virtual std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>
			Subscribe(UA_Client *client, ModelOpcUa::NodeId_t, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback);

			void createSubscription(UA_Client *client);

			void deleteSubscription(UA_Client *client);

			void setSubscriptionWrapper(Umati::OpcUa::OpcUaSubscriptionInterface *pSubscriptionWrapper);

		protected:
			std::shared_ptr<UA_SessionState> _pSession;

			friend class ValueSubscriptionHandle;

			void Unsubscribe(UA_Client *client, UA_Int32 monItemId, UA_Int32 clientHandle);

			const std::map<std::string, uint16_t> &m_uriToIndexCache;
			const std::map<uint16_t, std::string> &m_indexToUriCache;
			static std::atomic_uint nextId;
			UA_CreateSubscriptionRequest *m_pSubscription = nullptr;
			UA_DeleteSubscriptionsRequest *m_pDeleteSubscription = nullptr;
			UA_Int32 m_pSubscriptionID;
			Umati::OpcUa::OpcUaSubscriptionInterface *m_pSubscriptionWrapper = new OpcUaSubscriptionWrapper();

			std::map<UA_Int32, Dashboard::IDashboardDataClient::newValueCallbackFunction_t> m_callbacks;

			UA_MonitoredItemCreateRequest &
			prepareMonItemCreateReq(const ModelOpcUa::NodeId_t &nodeId,
									UA_MonitoredItemCreateRequest &monItemCreateReq) const;

			static void
            validateMonitorItemResult(const UA_StatusCode &uaResult, UA_MonitoredItemCreateResult monItemCreateResult,
									  const ModelOpcUa::NodeId_t &nodeId);
		};

	}
}
