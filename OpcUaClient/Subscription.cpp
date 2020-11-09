#include "Subscription.hpp"
#include <easylogging++.h>
#include <uasession.h>
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

			ValueSubscriptionHandle(Subscription *pSubscription, OpcUa_UInt32 monItemId, OpcUa_UInt32 clientHandle)
					: m_monitoredItemId(monItemId), m_clientHandle(clientHandle), m_pClientSubscription(pSubscription) {

			}

			virtual ~ValueSubscriptionHandle() {
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
				LOG(INFO) << "Unsubscribing monitoredItemId:" << std::to_string(m_monitoredItemId);
				m_pClientSubscription->Unsubscribe(m_monitoredItemId, m_clientHandle);
				this->setUnsubscribed();
			}

			const OpcUa_UInt32 m_monitoredItemId;
			const OpcUa_UInt32 m_clientHandle;

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

		void Subscription::subscriptionStatusChanged(OpcUa_UInt32 clientSubscriptionHandle, const UaStatus &status) {
			OpcUa_ReferenceParameter(clientSubscriptionHandle); // We use the callback only for this subscription

			std::stringstream str;
			str << "SubscriptionStatus changed to " << status.toString().toUtf8();
			LOG(WARNING) << str.str().c_str();
			if (status.isBad()) {
				// recover subscription
				LOG(WARNING) << "Deleting subscription " << this;
				deleteSubscription(_pSession);
				createSubscription(_pSession);
				LOG(WARNING) << "deleted subscription";
			}
		}

		void Subscription::dataChange(OpcUa_UInt32 /*clientSubscriptionHandle*/,
									  const UaDataNotifications &dataNotifications,
									  const UaDiagnosticInfos & /*diagnosticInfos*/) {
			for (OpcUa_UInt32 i = 0; i < dataNotifications.length(); ++i) {
				auto clientHandle = dataNotifications[i].ClientHandle;
				auto it = m_callbacks.find(clientHandle);
				if (it == m_callbacks.end()) {
					LOG(WARNING) << "Received Item with unknown client handle.";
					continue;
				}

				auto value = Converter::UaDataValueToJsonValue(UaDataValue(dataNotifications[i].Value),
															   false).getValue();
				it->second(value);
			}
		}

		void
		Subscription::newEvents(OpcUa_UInt32 /*clientSubscriptionHandle*/, UaEventFieldLists & /*eventFieldList*/) {
			LOG(ERROR) << "Received new Event, Not implemented.";
		}

		void Subscription::createSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession) {
			UaClientSdk::ServiceSettings servSettings;
			UaClientSdk::SubscriptionSettings subSettings;
			_pSession = pSession;
			if (m_pSubscription == NULL) {
				auto result = m_pSubscriptionWrapper->SessionCreateSubscription(_pSession, servSettings, this, 1,
																				subSettings, OpcUa_True,
																				&m_pSubscription);
				if (!result.isGood()) {
					LOG(ERROR) << "Creation of the subscription failed: " << result.toString().toUtf8();
					/// \todo throw exception
				}
			} else {
				LOG(WARNING) << "Subscription is not empty, won't create new subscription.";
			}
		}

		void Subscription::deleteSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession) {
			if (m_pSubscription) {
				UaClientSdk::ServiceSettings servsettings;
				m_pSubscriptionWrapper->SessionDeleteSubscription(pSession, servsettings, &m_pSubscription);
				m_pSubscription = nullptr;
			}
		}

		void Subscription::Unsubscribe(OpcUa_UInt32 monItemId, OpcUa_UInt32 clientHandle) {
			auto it = m_callbacks.find(clientHandle);
			if (it != m_callbacks.end()) {
				m_callbacks.erase(clientHandle);
			} else {
				LOG(WARNING) << "No callback found for client handle " << clientHandle;
			}

			UaStatusCodeArray results;
			UaUInt32Array monItems;

			monItems.resize(1);
			monItems[0] = monItemId;
			UaClientSdk::ServiceSettings servSettings;
			// the next call causes a segv => cover with unittest

			if (m_pSubscription == NULL) {
				createSubscription(_pSession);
			}

			if (m_pSubscription != NULL) {
				LOG(WARNING) << "deleting monitored items, subscription used: " << this
							 << ". is this the old one? is this allowed to be the old one?";
				LOG(WARNING) << "deleting monitored items, uasubscription used: " << m_pSubscription
							 << ". is this the old one? is this allowed to be the old one?";
				auto ret = m_pSubscription->deleteMonitoredItems(
						servSettings,
						monItems,
						results
				);

				if (ret.isNotGood()) {
					LOG(WARNING) << "Removal of subscribed item failed: " << ret.toString().toUtf8();
				}

				if (results.length() == 1) {
					UaStatusCode resultCode(results[0]);
					if (resultCode.isNotGood()) {
						LOG(WARNING) << "Removal of subscribed item failed : " << resultCode.toString().toUtf8();
					}
				} else {
					LOG(ERROR) << "Length mismatch, unsubscribe might have failed.";
				}
			} else {
				LOG(ERROR) << "UaSubscription m_pSubscription is null, can't execute.";
			}
		}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> Subscription::Subscribe(
				ModelOpcUa::NodeId_t nodeId,
				Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback
		) {
			// LOG(INFO) << "Subscribe request for nodeId " << nodeId.Uri << ";" << nodeId.Id;
			UaClientSdk::ServiceSettings servSettings;
			UaMonitoredItemCreateRequests monItemCreateReq;
			UaMonitoredItemCreateResults monItemCreateResult;

			monItemCreateReq = prepareMonItemCreateReq(nodeId, monItemCreateReq);

			try {
				auto uaResult = m_pSubscription->createMonitoredItems(
						servSettings,
						OpcUa_TimestampsToReturn_Source,
						monItemCreateReq,
						monItemCreateResult
				);

				validateMonitorItemResult(uaResult, monItemCreateResult, nodeId);

				// LOG(INFO) << "Created monItemCreateReq for with clientHandle "<< monItemCreateReq[0].RequestedParameters.ClientHandle << " for the callback method.";
				m_callbacks.insert(std::make_pair(monItemCreateReq[0].RequestedParameters.ClientHandle, callback));
				return std::make_shared<ValueSubscriptionHandle>(this, monItemCreateResult[0].MonitoredItemId,
																 monItemCreateReq[0].RequestedParameters.ClientHandle);
			}
			catch (std::exception &ex) {
				throw ex;
			}
		}

		UaMonitoredItemCreateRequests &Subscription::prepareMonItemCreateReq(const ModelOpcUa::NodeId_t &nodeId,
																			 UaMonitoredItemCreateRequests &monItemCreateReq) const {
			monItemCreateReq.create(1);
			monItemCreateReq[0].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
			monItemCreateReq[0].MonitoringMode = OpcUa_MonitoringMode_Reporting;
			monItemCreateReq[0].RequestedParameters.ClientHandle = nextId++;
			monItemCreateReq[0].RequestedParameters.SamplingInterval = 300;
			monItemCreateReq[0].RequestedParameters.QueueSize = 1;
			monItemCreateReq[0].RequestedParameters.DiscardOldest = OpcUa_True;
			Converter::ModelNodeIdToUaNodeId(nodeId, m_uriToIndexCache)
					.getNodeId().copyTo(&monItemCreateReq[0].ItemToMonitor.NodeId);
			return monItemCreateReq;
		}

		void
		Subscription::validateMonitorItemResult(UaStatus uaResult, UaMonitoredItemCreateResults monItemCreateResult,
												ModelOpcUa::NodeId_t nodeId) {
			if (uaResult.isBad()) {
				LOG(ERROR) << "Create Monitored items for " << nodeId.Uri << ";" << nodeId.Uri << " failed with: "
						   << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (monItemCreateResult.length() != 1) {
				LOG(ERROR) << "Expect monItemCreateResult.length() == 1 for " << nodeId.Uri << ";" << nodeId.Uri
						   << " , got:" << monItemCreateResult.length();
				throw Exceptions::UmatiException("Length mismatch.");
			}

			auto uaResultMonItem = UaStatusCode(monItemCreateResult[0].StatusCode);
			if (uaResultMonItem.isBad()) {
				LOG(ERROR) << "Monitored Item status code bad for " << nodeId.Uri << ";" << nodeId.Uri << " : "
						   << uaResultMonItem.toString().toUtf8();
			}
		}
	}
}
