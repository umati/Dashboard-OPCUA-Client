#include "PublishTopicFactory.hpp"

#include <easylogging++.h>
#include <sstream>
#include <regex>

namespace Umati {
	namespace MachineObserver {

		PublishTopicFactory::PublishTopicFactory()
		{
		}

		Umati::Util::PublishTopics PublishTopicFactory::getPubTopics(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machineRoot)
		{

			std::regex pattern("^([a-z0-9A-Z]+)-([a-z0-9A-Z]+)$");
			
			std::smatch match;
			bool found = std::regex_search(machineRoot.BrowseName.Name, match, pattern);

			std::string company = "NotSet";
			std::string machineName = "NotSet";
			if (!found)
			{
				LOG(ERROR) << "Could not determine company and machine name from: "
					<< machineRoot.BrowseName.Name << "(" << machineRoot.BrowseName.Uri << ")";
			}
			else
			{
				company = match[1].str();
				machineName = match[2].str();
			}

			std::stringstream ss;
			ss << "/" << company << "/" << machineName;
			std::string topicPrefix(ss.str());
			
			LOG(INFO) << "Using prefix '" << topicPrefix << "' for machine " << machineRoot.BrowseName.Uri;
			
			return Umati::Util::PublishTopics(topicPrefix);
		}
	}
}
