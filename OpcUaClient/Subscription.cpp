#include "Subscription.hpp"

#include <utility>
#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"


namespace Umati {
	namespace OpcUa {

		/***
		 * Used to handle unsubscribe of subscribedValues of each of the DashboardClients
		 */
		class ValueSubscriptionHandle : public Dashboard::IDashboardDataClient::ValueSubscriptionHandle {
		public:

			ValueSubscriptionHandle(Subscription *pSubscription, UA_Int32 monItemId, UA_Int32 clientHandle)
					: m_monitoredItemId(monItemId), m_clientHandle(clientHandle), m_pClientSubscription(pSubscription) {

			}
				~ValueSubscriptionHandle() override {
				unsubscribeInternal();
			}

			// Inherit from ValueSubscriptionHandle
			void unsubscribe() override {
				this->unsubscribeInternal();
			}

		protected:
			/// Unsubscribe the value, non virtual function, so it's safe to call it in the destructor.
			void unsubscribeInternal() {
				if (isUnsubscribed()) {
					return;
				}
				if (m_pClientSubscription == NULL) {
					LOG(ERROR) << "clientSubscription is null, cant unsubscribe";
					this->setUnsubscribed();
					return;
				}
				//TODO get client object
				//m_pClientSubscription->Unsubscribe(client, m_monitoredItemId, m_clientHandle);
				this->setUnsubscribed();
			}

			const UA_Int32 m_monitoredItemId;
			const UA_Int32 m_clientHandle;

			Subscription *m_pClientSubscription;
		};

		std::atomic_uint Subscription::nextId = {100};

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
			//FIXME compiler error
			//UA_ReferenceTypeAttributes(clientSubscriptionHandle);// We use the callback only for this subscription
			std::stringstream str;
			str << "SubscriptionStatus changed to " << status;
			LOG(WARNING) << str.str().c_str();
			if (UA_StatusCode_isBad(status)) {
				// recover subscription
				LOG(WARNING) << "Deleting subscription " << this;
				deleteSubscription(client,_pSession);
				createSubscription(client, _pSession);
				LOG(WARNING) << "deleted subscription";
			}
		}

		void Subscription::dataChange(UA_Int32 /*clientSubscriptionHandle*/,
									  const UA_DataChangeNotification &dataNotifications,
									  const UA_DiagnosticInfo & /*diagnosticInfos*/) {
			for (UA_Int32 i = 0; i < dataNotifications.diagnosticInfosSize; ++i) {
				auto clientHandle = dataNotifications.monitoredItems->clientHandle;
				auto it = m_callbacks.find(clientHandle);
				if (it == m_callbacks.end()) {
					LOG(WARNING) << "Received Item with unknown client handle.";
					continue;
				}

				auto value = Converter::UaDataValueToJsonValue(UA_DataValue(dataNotifications.monitoredItems->value),
															   false).getValue();
				it->second(value);
			}
		}

		void
		Subscription::newEvents(UA_Int32 clientSubscriptionHandle , UA_EventFieldList & eventFieldList) {
			LOG(ERROR) << "Received new Event, Not implemented.";
		}

		void Subscription::createSubscription(UA_Client *client, std::shared_ptr<UA_SessionState> pSession) {
			//TODO find service settings datatype in open62541
			//UaClientSdk::ServiceSettings servSettings;
			//UaClientSdk::SubscriptionSettings subSettings;
			_pSession = std::move(pSession);
			if (m_pSubscription == NULL) {
				auto request = UA_CreateSubscriptionRequest_default();
				auto result = m_pSubscriptionWrapper->SessionCreateSubscription(client, request,
                                                                            NULL, NULL, NULL);
    			if(!UA_StatusCode_isBad(result.responseHeader.serviceResult)){
        			LOG(ERROR) << "Create subscription succeeded, id " << result.subscriptionId;
					m_pSubscriptionID = result.subscriptionId;
				} else {
				LOG(WARNING) << "Subscription is not empty, won't create new subscription.";	
			} 
		  }
		}
		//TODO find according datatype for session
		void Subscription::deleteSubscription(UA_Client *client, std::shared_ptr<UA_SessionState> pSession) {
			if (m_pSubscription) {
				//TODO find service settings datatype in open62541 and use session state?
				// UaClientSdk::ServiceSettings servsettings;
				m_pSubscriptionWrapper->SessionDeleteSubscription(client, /*servsettings, &m_pSubscription*/ *m_pDeleteSubscription);
				m_pSubscription = nullptr;
			}
		}

		void Subscription::Unsubscribe(UA_Client *client, UA_Int32 monItemId, UA_Int32 clientHandle) {
			auto it = m_callbacks.find(clientHandle);
			if (it != m_callbacks.end()) {
				m_callbacks.erase(clientHandle);
			} else {
				LOG(WARNING) << "No callback found for client handle " << clientHandle;
			}

			UA_StatusCode results;
			UA_Int32 monItems;
			monItems = monItemId;
			//TODO find service settings datatype in open62541
			//UaClientSdk::ServiceSettings servSettings;
			// the next call causes a segv => cover with unittest

			
			if (m_pSubscription == NULL) {
				createSubscription(client, _pSession);
			}
			//TODO use other datatype for m_pSubscription. See header. 
			UA_DeleteSubscriptionsResponse response;
			response = UA_Client_Subscriptions_delete(client, *m_pDeleteSubscription);
			results = *response.results;
			//VERIFY what to use?
			//results = UA_Client_MonitoredItems_deleteSingle(client, m_pSubscriptionID, monItemId);
			/*
			if (m_pSubscription != NULL) {
				auto ret = m_pSubscription->deleteMonitoredItems(
						servSettings,
						monItems,
						results
				);
			*/
				if (UA_StatusCode_isBad(results)) {
					LOG(WARNING) << "Removal of subscribed item failed: " << results;
				}
				//VERIFY double check on same object?
				if (response.resultsSize == 1) {
					UA_StatusCode resultCode(results);
					if (UA_StatusCode_isBad(resultCode)) {
						LOG(WARNING) << "Removal of subscribed item failed : " << resultCode;
					}
				} else {
					LOG(ERROR) << "Length mismatch, unsubscribe might have failed.";
				}
			/*} else {
				LOG(ERROR) << "UaSubscription m_pSubscription is null, can't execute.";
			} */
		}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> Subscription::Subscribe(
				UA_Client *client,
				ModelOpcUa::NodeId_t nodeId,
				Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback
		) {
			 LOG(INFO) << "Subscribe request for nodeId " << nodeId.Uri << ";" << nodeId.Id;
			//TODO service Settings
			//UaClientSdk::ServiceSettings servSettings;
			UA_MonitoredItemCreateRequest monItemCreateReq;
			UA_MonitoredItemCreateResult monItemCreateResult;

			monItemCreateReq = prepareMonItemCreateReq(nodeId, monItemCreateReq);
			//TODO find replacement for uaSubscription. See header
			//TODO callback conversion from callback parameter to _callback?
			UA_Client_DataChangeNotificationCallback _callback;
			
			try {
			//FIXME SEGV
			monItemCreateResult = UA_Client_MonitoredItems_createDataChange(client,m_pSubscriptionID,UA_TIMESTAMPSTORETURN_SOURCE,monItemCreateReq,NULL, _callback, NULL);

			validateMonitorItemResult(monItemCreateResult.statusCode, monItemCreateReq, nodeId);

				// LOG(INFO) << "Created monItemCreateReq for with clientHandle "<< monItemCreateReq[0].RequestedParameters.ClientHandle << " for the callback method.";
			//TODO wrong callback object is stored...
			m_callbacks.insert(std::make_pair(monItemCreateReq.requestedParameters.clientHandle, callback));
			return std::make_shared<ValueSubscriptionHandle>(this, monItemCreateResult.monitoredItemId,
																 monItemCreateReq.requestedParameters.clientHandle);
			}
			catch (std::exception &ex) {
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
												UA_MonitoredItemCreateRequest monItemCreateResult,
												const ModelOpcUa::NodeId_t &nodeId) {
			if  (UA_StatusCode_isBad(uaResult)){
				LOG(ERROR) << "Create Monitored items for " << nodeId.Uri << ";" << nodeId.Id << " failed with: "
						   <<  UA_StatusCode_name(uaResult);
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (monItemCreateResult.requestedParameters.queueSize != 1) {
				LOG(ERROR) << "Expect monItemCreateResult.length() == 1 for " << nodeId.Uri << ";" << nodeId.Uri
						   << " , got:" << monItemCreateResult.requestedParameters.queueSize;
				throw Exceptions::UmatiException("Length mismatch.");
			}
			//FIXME get statuscode from result
			auto uaResultMonItem = UA_StatusCode(monItemCreateResult.itemToMonitor.attributeId);
			if (UA_StatusCode_isBad(uaResultMonItem)) {
				LOG(ERROR) << "Monitored Item status code bad for " << nodeId.Uri << ";" << nodeId.Uri << " : "
						   << uaResultMonItem;
			}
		}
	}
}
