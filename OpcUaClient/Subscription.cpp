#include "Subscription.hpp"
#include <easylogging++.h>
#include <uasession.h>
#include "Converter/ModelNodeIdToUaNodeId.hpp"
#include "Converter/UaDataValueToJsonValue.hpp"
#include "Exceptions/OpcUaNonGoodStatusCodeException.hpp"

namespace Umati {
	namespace OpcUa {

		class ValueSubscriptionHandle : public Dashboard::IDashboardDataClient::ValueSubscriptionHandle {
		public:
			/// \todo reduce to one parameter
			ValueSubscriptionHandle(Subscription *pSubscription, OpcUa_UInt32 monItemId, OpcUa_UInt32 clientHandle)
			: m_monitoredItemId(monItemId), m_clientHandle(clientHandle), m_pClientSubscription(pSubscription)
			{

			}

			virtual ~ValueSubscriptionHandle()
			{
				unsubscribeInternal();
			}

			// Inherit from ValueSubscriptionHandle
			void unsubscribe() override
			{
				this->unsubscribeInternal();
			}

		protected:
			/// Ubsubscribe the value, non virtual function, so it's safe to call it in the destructor.
			void unsubscribeInternal()
			{
				if (isUnsubscribed())
				{
					return;
				}

				m_pClientSubscription->Unsubscribe(m_monitoredItemId, m_clientHandle);
				this->setUnsubscribed();
			}

			const OpcUa_UInt32 m_monitoredItemId;
			const OpcUa_UInt32 m_clientHandle;

			Subscription *m_pClientSubscription;
		};

		std::atomic_uint Subscription::nextId = {100};

		Subscription::Subscription(
			const std::map<std::string, uint16_t>& uriToIndexCache,
			const std::map<uint16_t, std::string>& indexToUriCache
		)
			:m_uriToIndexCache(uriToIndexCache), m_indexToUriCache(indexToUriCache)
		{

		}

		void Subscription::subscriptionStatusChanged(OpcUa_UInt32 clientSubscriptionHandle, const UaStatus & status)
		{
			OpcUa_ReferenceParameter(clientSubscriptionHandle); // We use the callback only for this subscription

			std::stringstream str;
			str << "SubscriptionStatus changed to " << status.toString().toUtf8();
			LOG(WARNING) << str.str().c_str();

			// recover subscription
			deleteSubscription(_pSession);
			createSubscription(_pSession);
		}

		void Subscription::dataChange(OpcUa_UInt32 /*clientSubscriptionHandle*/, const UaDataNotifications & dataNotifications, const UaDiagnosticInfos & /*diagnosticInfos*/)
		{
			for (OpcUa_UInt32 i = 0; i < dataNotifications.length(); ++i)
			{
				auto clientHandle = dataNotifications[i].ClientHandle;
				auto it = m_callbacks.find(clientHandle);
				if (it == m_callbacks.end())
				{
					LOG(WARNING) << "Received Item with unknown client handle.";
					continue;
				}

				auto value = Converter::UaDataValueToJsonValue(UaDataValue(dataNotifications[i].Value)).getValue();
				it->second(value);
			}
		}

		void Subscription::newEvents(OpcUa_UInt32 /*clientSubscriptionHandle*/, UaEventFieldLists & /*eventFieldList*/)
		{
			LOG(ERROR) << "Received new Event, Not implemented.";
		}

		void Subscription::createSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession)
		{
			UaClientSdk::ServiceSettings servSettings;
			UaClientSdk::SubscriptionSettings subSettings;
			_pSession = pSession;
			auto result = pSession.get()->createSubscription(servSettings, this, 1, subSettings, OpcUa_True, &m_pSubscription);
			if (!result.isGood())
			{
				LOG(ERROR) << "Creation of the subscription failed: " << result.toString().toUtf8();
				/// \todo throw exception
			}
		}

		void Subscription::deleteSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession)
		{
			if (m_pSubscription)
			{
				UaClientSdk::ServiceSettings servsettings;
				pSession->deleteSubscription(servsettings, &m_pSubscription);
				m_pSubscription = nullptr;
			}
		}

		void Subscription::Unsubscribe(OpcUa_UInt32 monItemId, OpcUa_UInt32 clientHandle)
		{
			m_callbacks.erase(clientHandle);

			UaStatusCodeArray results;
			UaUInt32Array monItems;

			monItems.resize(1);
			monItems[0] = monItemId;
			UaClientSdk::ServiceSettings servSettings;
			auto ret = m_pSubscription->deleteMonitoredItems(
				servSettings,
				monItems,
				results
			);

			if (ret.isNotGood())
			{
				LOG(WARNING) << "Removal of subscribed item failed: " << ret.toString().toUtf8();
			}

			if (results.length() == 1)
			{
				UaStatusCode resultCode(results[0]);
				if (resultCode.isNotGood())
				{
					LOG(WARNING) << "Removal of subscribed item failed : " << resultCode.toString().toUtf8();
				}
			}
			else
			{
				LOG(ERROR) << "Length mismatch, unsubscribe might have failed.";
			}
		}

		std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> Subscription::Subscribe(
			ModelOpcUa::NodeId_t nodeId,
			Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback
		)
		{
			UaClientSdk::ServiceSettings servSettings;
			UaMonitoredItemCreateRequests monItemCreateReq;
			UaMonitoredItemCreateResults monItemCreateResult;

			monItemCreateReq.create(1);
			monItemCreateReq[0].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
			monItemCreateReq[0].MonitoringMode = OpcUa_MonitoringMode_Reporting;
			monItemCreateReq[0].RequestedParameters.ClientHandle = nextId++;
			monItemCreateReq[0].RequestedParameters.SamplingInterval = 300;
			monItemCreateReq[0].RequestedParameters.QueueSize = 1;
			monItemCreateReq[0].RequestedParameters.DiscardOldest = OpcUa_True;
			Converter::ModelNodeIdToUaNodeId(nodeId, m_uriToIndexCache)
				.getNodeId().copyTo(&monItemCreateReq[0].ItemToMonitor.NodeId);


			auto uaResult = m_pSubscription->createMonitoredItems(
				servSettings,
				OpcUa_TimestampsToReturn_Source,
				monItemCreateReq,
				monItemCreateResult
			);

			
			if (uaResult.isBad())
			{
				LOG(ERROR) << "Create Monitored items failed with: " << uaResult.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResult);
			}

			if (monItemCreateResult.length() != 1)
			{
				LOG(ERROR) << "Expect monItemCreateResult.length() == 1, got:" << monItemCreateResult.length();
				throw Exceptions::UmatiException("Length mismatch.");
			}

			auto uaResultMonItem = UaStatusCode(monItemCreateResult[0].StatusCode);
			if (uaResultMonItem.isBad())
			{
				LOG(ERROR) << "Monitored Item status code bad: " << uaResultMonItem.toString().toUtf8();
				throw Exceptions::OpcUaNonGoodStatusCodeException(uaResultMonItem);
			}

			m_callbacks.insert(std::make_pair(monItemCreateReq[0].RequestedParameters.ClientHandle, callback));

			return std::make_shared<ValueSubscriptionHandle>(
				this,
				monItemCreateResult[0].MonitoredItemId, monItemCreateReq[0].RequestedParameters.ClientHandle
			);
		}

	}
}
