#pragma once

#include <uaclientsdk.h>
#include <uasubscription.h>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <atomic>
#include <IDashboardDataClient.hpp>

namespace Umati {
	namespace OpcUa {
		class Subscription : public UaClientSdk::UaSubscriptionCallback
		{
		public:
			Subscription(const std::map<std::string, uint16_t> &m_uriToIndexCache, const std::map<uint16_t, std::string> &m_indexToUriCache);

			void subscriptionStatusChanged(OpcUa_UInt32 clientSubscriptionHandle, const UaStatus & status) override;
			void dataChange(OpcUa_UInt32 clientSubscriptionHandle, const UaDataNotifications & dataNotifications, const UaDiagnosticInfos & diagnosticInfos) override;
			void newEvents(OpcUa_UInt32 clientSubscriptionHandle, UaEventFieldLists & eventFieldList) override;

			virtual std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> Subscribe(ModelOpcUa::NodeId_t, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback);

			void createSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession);
			void deleteSubscription(std::shared_ptr<UaClientSdk::UaSession> pSession);

		protected:
			std::shared_ptr<UaClientSdk::UaSession> _pSession;

			friend class ValueSubscriptionHandle;

			void Unsubscribe(OpcUa_UInt32 monItemId, OpcUa_UInt32 clientHandle);

			const std::map<std::string, uint16_t> &m_uriToIndexCache;
			const std::map<uint16_t, std::string> &m_indexToUriCache;
			static std::atomic_uint nextId;

			UaClientSdk::UaSubscription *m_pSubscription = nullptr;

			std::map<OpcUa_UInt32, Dashboard::IDashboardDataClient::newValueCallbackFunction_t> m_callbacks;
		};

	}
}
