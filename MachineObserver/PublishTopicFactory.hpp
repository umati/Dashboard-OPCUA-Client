#pragma once

#include <PublishTopics.hpp>
#include <IDashboardDataClient.hpp>

namespace Umati {
	namespace MachineObserver {
		class PublishTopicFactory
		{
		public:
			PublishTopicFactory();

			Umati::Util::PublishTopics getPubTopics(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machineRoot);
		protected:
		};
	}
}
