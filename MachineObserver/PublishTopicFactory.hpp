#pragma once

#include <PublishTopics.hpp>
#include <IDashboardDataClient.hpp>
#include "IMachineCache.hpp"

namespace Umati {
	namespace MachineObserver {
		class PublishTopicFactory
		{
		public:
			PublishTopicFactory(std::shared_ptr<IMachineCache> pMachineCache);

			Umati::Util::PublishTopics getPubTopics(ModelOpcUa::BrowseResult_t machineRoot);
		protected:
		    std::shared_ptr<IMachineCache> m_pMachineCache;
		};
	}
}
