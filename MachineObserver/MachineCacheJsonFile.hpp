#pragma once

#include "IMachineCache.hpp"
#include <map>

namespace Umati {
	namespace MachineObserver {
		class MachineCacheJsonFile : public IMachineCache
		{
		public:

			const std::string JsonKey_Uri = std::string("Uri");
			const std::string JsonKey_TopicPrefix = std::string("TopicPrefix");

			MachineCacheJsonFile(std::string cacheFilename, bool autosave=true);

			~MachineCacheJsonFile() = default;

			// Inherit from IMachineCache
			void AddEntry(MachineCacheEntry_t entry) override;
			std::shared_ptr<MachineCacheEntry_t> GetEntry(std::string NamespaceURI) override;
			const bool AutoSave;

			void saveJson();

		protected:

			void readJson();

			std::string m_cacheFilename;
			std::map<std::string, IMachineCache::MachineCacheEntry_t> m_cache;
		};
	}
}
