#pragma once

#include <string>
#include <memory>

namespace Umati {
	namespace MachineObserver {
		class IMachineCache
		{
		public:
			virtual ~IMachineCache() = 0;

			struct MachineCacheEntry_t {
				std::string NamespaceUri;
				std::string TopicPrefix;
			};

			virtual std::shared_ptr<MachineCacheEntry_t> GetEntry(std::string NamespaceURI) = 0;
			
			virtual void AddEntry(MachineCacheEntry_t entry) = 0;
		};
	}
}
