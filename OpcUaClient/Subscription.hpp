 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2021 (c) Frank Meerkoetter, basysKom GmbH
 */

#pragma once

#include <open62541/client.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <Open62541Cpp/UA_NodeId.hpp>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <atomic>
#include <IDashboardDataClient.hpp>
#include "OpcUaSubscriptionInterface.hpp"
#include <mutex>

namespace Umati {
	namespace OpcUa  {
		class Subscription {
		public:

			~Subscription();

			Subscription(const std::map<std::string, uint16_t> &m_uriToIndexCache,
						 const std::map<uint16_t, std::string> &m_indexToUriCache);

			void subscriptionStatusChanged(UA_Client *client,UA_Int32 clientSubscriptionHandle, const UA_StatusCode &status);

			void dataChange(UA_Int32 clientSubscriptionHandle, const UA_DataChangeNotification &dataNotifications,
							const UA_DiagnosticInfo &diagnosticInfos, UA_Client *client, UA_NodeId nid);

			void newEvents(UA_Int32 clientSubscriptionHandle, UA_EventFieldList &eventFieldList); 

			virtual std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>
			Subscribe(UA_Client *client, ModelOpcUa::NodeId_t, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback);

			void Unsubscribe(UA_Client *client, std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles);

			void createSubscription(UA_Client *client);

			void deleteSubscription(UA_Client *client);

			void setSubscriptionWrapper(Umati::OpcUa::OpcUaSubscriptionInterface *pSubscriptionWrapper);

			std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> valueSubscriptionHandle;
			const std::map<std::string, uint16_t> &m_uriToIndexCache;

		protected:
			std::shared_ptr<UA_SessionState> _pSession;

			friend class ValueSubscriptionHandle;
			
			const std::map<uint16_t, std::string> &m_indexToUriCache;
			static std::atomic_uint nextId;
			UA_Int32 m_pSubscriptionID;
			Umati::OpcUa::OpcUaSubscriptionInterface *m_pSubscriptionWrapper = new OpcUaSubscriptionWrapper();

			std::mutex m_callbacks_mutex;
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
