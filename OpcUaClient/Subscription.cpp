 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "Subscription.hpp"

#include <utility>
#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"
#include "Converter/UaNodeIdToModelNodeId.hpp"
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

static void createDataChangeCallback(UA_Client *client, UA_UInt32 subId, void *subContext,
               			UA_UInt32 monId, void *monContext, UA_DataValue *dataValue)
{
  //LOG(INFO) << "createDataChangeCallback " << subContext << " | " << monContext<< " | "<<dataValue;
  auto* sub = (Umati::OpcUa::Subscription*)subContext;
  UA_MonitoredItemNotification monitems;
  UA_MonitoredItemNotification_init(&monitems);
  int handle = static_cast<int>(reinterpret_cast<std::uintptr_t>(monContext));
  monitems.clientHandle = (UA_Int32) handle;
  monitems.value = *dataValue;
  UA_DataChangeNotification notify;
  UA_DataChangeNotification_init(&notify);
  notify.monitoredItems = &monitems;
  notify.monitoredItemsSize = 1;
  auto nodeId = sub->valueSubscriptionHandle.get()->getNodeId();
  auto uanodeId = (open62541Cpp::UA_NodeId)(Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, sub->m_uriToIndexCache)
					.getNodeId());
  sub->dataChange(monId, notify, *notify.diagnosticInfos, client, *uanodeId.NodeId);
} 

namespace Umati {
	namespace OpcUa {

		std::atomic_uint Subscription::nextId = {1};

		Subscription::~Subscription(){
			delete m_pSubscriptionWrapper;
			m_pSubscriptionWrapper = NULL;
		}

		Subscription::Subscription(
				const std::map<std::string, uint16_t> &uriToIndexCache,
				const std::map<uint16_t, std::string> &indexToUriCache
		)
				: m_uriToIndexCache(uriToIndexCache), m_indexToUriCache(indexToUriCache) {
			LOG(WARNING) << "Created subscription " << this;
		}

		void Subscription::setSubscriptionWrapper(OpcUaSubscriptionInterface *pSubscriptionWrapper) {
			m_pSubscriptionWrapper = pSubscriptionWrapper;
		}

		void Subscription::subscriptionStatusChanged(UA_Client *client,UA_Int32 clientSubscriptionHandle, const UA_StatusCode &status) {
			std::stringstream str;
			str << "SubscriptionStatus changed to " << status;
			LOG(WARNING) << str.str().c_str();
			if (UA_StatusCode_isBad(status)) {
				// recover subscription
				LOG(WARNING) << "Deleting subscription " << this;
				deleteSubscription(client);
				createSubscription(client);
				LOG(WARNING) << "deleted subscription";
			}
		}

		void Subscription::dataChange(UA_Int32 /*clientSubscriptionHandle*/,
									  const UA_DataChangeNotification &dataNotifications,
									  const UA_DiagnosticInfo & /*diagnosticInfos*/,
									  UA_Client *client, UA_NodeId nid) {
			std::unique_lock<decltype(m_callbacks_mutex)> ul(m_callbacks_mutex);
			for (UA_Int32 i = 0; i < dataNotifications.monitoredItemsSize; ++i) {
				auto clientHandle = dataNotifications.monitoredItems->clientHandle;
				auto it = m_callbacks.find(clientHandle);
				if (it == m_callbacks.end()) {
					LOG(WARNING) << "Received Item with unknown client handle.";
					continue;
				}

				auto value = Converter::UaDataValueToJsonValue(UA_DataValue(dataNotifications.monitoredItems->value),
															   client, nid,
															   false).getValue();
				it->second(value);
			}
		}

		void
		Subscription::newEvents(UA_Int32 clientSubscriptionHandle , UA_EventFieldList & eventFieldList) {
			LOG(ERROR) << "Received new Event, Not implemented.";
		}

		void Subscription::createSubscription(UA_Client *client) {
				auto request = UA_CreateSubscriptionRequest_default();
				auto result = m_pSubscriptionWrapper->SessionCreateSubscription(client, request,
																				this, NULL, NULL);
				if(!UA_StatusCode_isBad(result.responseHeader.serviceResult)){
					LOG(ERROR) << "Create subscription succeeded, id " << result.subscriptionId;
					m_pSubscriptionID = result.subscriptionId;
				} else {
					LOG(WARNING) << "Subscription is not empty, won't create new subscription.";	
				}
		}

		void Subscription::deleteSubscription(UA_Client *client) {
				m_pSubscriptionWrapper->SessionDeleteSubscription(client, m_pSubscriptionID);
		}

		void Subscription::Unsubscribe(UA_Client *client, std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles) {
			{
				std::unique_lock<decltype(m_callbacks_mutex)> ul(m_callbacks_mutex);
				for(UA_Int32 handle : clientHandles){
					auto it = m_callbacks.find(handle);
					if (it != m_callbacks.end()) {
						m_callbacks.erase(handle);
					} else {
						LOG(WARNING) << "No callback found for client handle " << handle;
					}
				}
			}

			const size_t monItemIdsSize = monItemIds.size();
			UA_UInt32 *newMonitoredItemIds = (UA_UInt32 *) UA_Array_new(monItemIdsSize, &UA_TYPES[UA_TYPES_UINT32]);
			
			for (int i = 0; i < monItemIdsSize; i++){
				newMonitoredItemIds[i] = (UA_UInt32)clientHandles.at(i);
			}

			UA_DeleteMonitoredItemsRequest deleteRequest;
    		UA_DeleteMonitoredItemsRequest_init(&deleteRequest);
			deleteRequest.monitoredItemIdsSize = monItemIdsSize;
			deleteRequest.monitoredItemIds = newMonitoredItemIds;
			deleteRequest.subscriptionId = m_pSubscriptionID;

			auto response = UA_Client_MonitoredItems_delete(client, deleteRequest);

			if (UA_StatusCode_isBad(response.responseHeader.serviceResult) || response.resultsSize != deleteRequest.monitoredItemIdsSize) {
				LOG(WARNING) << "Removal of subscribed item failed: " << UA_StatusCode_name(response.responseHeader.serviceResult);
			}

			for (int i = 0; i < deleteRequest.monitoredItemIdsSize; i++){
				if (UA_StatusCode_isBad(response.results[i])){
					LOG(WARNING) << "Removal of subscribed item failed: " << UA_StatusCode_name(response.results[i]);
				}
			}
			UA_DeleteMonitoredItemsResponse_clear(&response);
			UA_Array_delete(newMonitoredItemIds, monItemIdsSize, &UA_TYPES[UA_TYPES_UINT32]);

        }

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> Subscription::Subscribe(
				UA_Client *client,
				ModelOpcUa::NodeId_t nodeId,
				Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback
		) {
			// LOG(INFO) << "Subscribe request for nodeId " << nodeId.Uri << ";" << nodeId.Id;
			UA_MonitoredItemCreateRequest monItemCreateReq;
			UA_MonitoredItemCreateResult monItemCreateResult;

			monItemCreateReq = prepareMonItemCreateReq(nodeId, monItemCreateReq);
			try {
                monItemCreateResult = UA_Client_MonitoredItems_createDataChange(client, m_pSubscriptionID, UA_TIMESTAMPSTORETURN_SOURCE, monItemCreateReq,
                                                                                (void*)((UA_Int64)(monItemCreateReq.requestedParameters.clientHandle)), createDataChangeCallback, NULL);
				validateMonitorItemResult(monItemCreateResult.statusCode, monItemCreateResult, nodeId);

                //LOG(INFO) << "Created monItemCreateReq for with clientHandle "<< monItemCreateReq.requestedParameters.clientHandle << " for the callback method.";
				{
					std::unique_lock<decltype(m_callbacks_mutex)> ul(m_callbacks_mutex);
					m_callbacks.insert(std::make_pair(monItemCreateReq.requestedParameters.clientHandle, callback));
					
				}
                auto returnPointer = std::make_shared<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>(monItemCreateResult.monitoredItemId,
																 monItemCreateReq.requestedParameters.clientHandle, nodeId);
				valueSubscriptionHandle = returnPointer;
				UA_MonitoredItemCreateResult_clear(&monItemCreateResult);
				UA_MonitoredItemCreateRequest_clear(&monItemCreateReq);

				return returnPointer;
			}
			catch (std::exception &ex) {
				LOG(INFO) << "Excepttion in subscribe request for nodeId " << nodeId.Uri << ";" << nodeId.Id << ex.what();
				UA_MonitoredItemCreateResult_clear(&monItemCreateResult);
				UA_MonitoredItemCreateRequest_clear(&monItemCreateReq);
				throw ex;
			}
		}
		
		UA_MonitoredItemCreateRequest &Subscription::prepareMonItemCreateReq(const ModelOpcUa::NodeId_t &nodeId,
																			 UA_MonitoredItemCreateRequest &monItemCreateReq) const {
			UA_MonitoredItemCreateRequest_init(&monItemCreateReq);
			monItemCreateReq.itemToMonitor.attributeId = UA_ATTRIBUTEID_VALUE;
			monItemCreateReq.monitoringMode = UA_MONITORINGMODE_REPORTING;
			monItemCreateReq.requestedParameters.clientHandle = nextId++;
			monItemCreateReq.requestedParameters.samplingInterval = 300;
			monItemCreateReq.requestedParameters.queueSize = 1;
			monItemCreateReq.requestedParameters.discardOldest = UA_TRUE;
			open62541Cpp::UA_NodeId id = (open62541Cpp::UA_NodeId)(Converter::ModelNodeIdToUaNodeId(nodeId, m_uriToIndexCache)
					.getNodeId());
			UA_NodeId_copy(id.NodeId,&monItemCreateReq.itemToMonitor.nodeId);
			
					
			return monItemCreateReq;
		}

		void
		Subscription::validateMonitorItemResult(const UA_StatusCode &uaResult,
                                                UA_MonitoredItemCreateResult monItemCreateResult,
												const ModelOpcUa::NodeId_t &nodeId) {
			if  (UA_StatusCode_isBad(uaResult)){
				LOG(ERROR) << "Create Monitored items for " << nodeId.Uri << ";" << nodeId.Id << " failed with: "
						   <<  UA_StatusCode_name(uaResult);
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

            if (monItemCreateResult.revisedQueueSize != 1) {
				LOG(ERROR) << "Expect monItemCreateResult.length() == 1 for " << nodeId.Uri << ";" << nodeId.Uri
                           << " , got:" << monItemCreateResult.revisedQueueSize;
				throw Exceptions::UmatiException("Length mismatch.");
			}
            if (UA_StatusCode_isBad(monItemCreateResult.statusCode)) {
				LOG(ERROR) << "Monitored Item status code bad for " << nodeId.Uri << ";" << nodeId.Uri << " : "
                           << monItemCreateResult.statusCode;
			}
		}
	}
}
