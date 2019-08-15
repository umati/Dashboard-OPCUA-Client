#include "PublishTopicFactory.hpp"
#include "PublishTopicFactory.hpp"

#include <easylogging++.h>
#include <sstream>
#include <regex>

namespace Umati {
	namespace MachineObserver {

		PublishTopicFactory::PublishTopicFactory(
			std::shared_ptr<IMachineCache> pMachineCache
		)
			: m_pMachineCache(pMachineCache)
		{
		}

		Umati::Util::PublishTopics PublishTopicFactory::getPubTopics(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machineRoot)
		{
			auto pCache = m_pMachineCache->GetEntry(machineRoot.NodeId.Uri);
			std::string topicPrefix;
			if (pCache)
			{
				topicPrefix = pCache->TopicPrefix;
			}
			else
			{
				topicPrefix = getTopicPrefixFromBrowseName(machineRoot.BrowseName);
				m_pMachineCache->AddEntry(
					IMachineCache::MachineCacheEntry_t { machineRoot.NodeId.Uri , topicPrefix}
				);
			}

			//LOG(INFO) << "Using prefix '" << topicPrefix << "' for machine " << machineRoot.BrowseName.Uri;
			
			return Umati::Util::PublishTopics(topicPrefix);
		}

		std::string PublishTopicFactory::getTopicPrefixFromBrowseName(ModelOpcUa::QualifiedName_t browseName)
		{
			std::regex pattern("^([a-z0-9A-Z]+)-([a-z0-9A-Z]+)$");

			std::smatch match;
			bool found = std::regex_search(browseName.Name, match, pattern);

			std::string company = "<CompanyNotSet>";
			std::string machineName = "<MachineNameNotSet>";
			if (!found)
			{
				LOG(ERROR) << "Could not determine company and machine name from: "
					<< browseName.Name << "(" << browseName.Uri << ")";
			}
			else
			{
				company = match[1].str();
				machineName = match[2].str();
			}

			std::stringstream ss;
			ss << "/" << company << "/" << machineName;
			return ss.str();
		}
	}
}
