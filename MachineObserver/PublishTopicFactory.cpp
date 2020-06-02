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

		Umati::Util::PublishTopics PublishTopicFactory::getPubTopics(ModelOpcUa::BrowseResult_t machineRoot)
		{
			auto pCache = m_pMachineCache->GetEntry(machineRoot.NodeId.Uri);
			std::string topicPrefix;
			if (pCache)
			{
				topicPrefix = pCache->TopicPrefix;
			}
            auto checkPublishTopics = Umati::Util::PublishTopics(topicPrefix);
			if (!pCache || !checkPublishTopics.isValid())
			{
                // todo browse for IdentifictionType and get mapping for machineName and Manufacturer for specific type from json
                topicPrefix = ""; //getTopicPrefixFromBrowseName(machineRoot.BrowseName);

                m_pMachineCache->AddEntry(
					IMachineCache::MachineCacheEntry_t {machineRoot.NodeId.Uri, topicPrefix}
				);
			}
            auto publishTopics = Umati::Util::PublishTopics(topicPrefix);
			if(!publishTopics.isValid()) {
			    LOG(ERROR) << "Invalid topic prefix: " << publishTopics.TopicPrefix;
			}

			//LOG(INFO) << "Using prefix '" << topicPrefix << "' for machine " << machineRoot.BrowseName.Uri;
			
			return publishTopics;
		}
	}
}
