#include "MachineCacheJsonFile.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <easylogging++.h>
#include <Exceptions/ConfigurationException.hpp>
#include <iomanip>

namespace Umati {
	namespace MachineObserver {

		static nlohmann::json getValueOrException(nlohmann::json json, std::string key)
		{
			auto it = json.find(key);
			if (it == json.end())
			{
				std::stringstream ss;
				ss << "Key '" << key << "' not found in: " << json << std::endl;
				throw Util::Exception::ConfigurationException(ss.str().c_str());
			}
			return *it;
		}

		MachineCacheJsonFile::MachineCacheJsonFile(std::string cacheFilename, bool autosave)
			: m_cacheFilename(cacheFilename), AutoSave(autosave)
		{
			std::ifstream i(m_cacheFilename);
			if (i)
			{
				i.close();
				readJson();
			}
			else
			{
				LOG(INFO) << "File '" << m_cacheFilename << "' not found, empty one will be created.";
			}

		}
		
		void MachineCacheJsonFile::AddEntry(MachineCacheEntry_t entry)
		{
			m_cache.insert(std::make_pair(entry.NamespaceUri, entry));
			if (AutoSave)
			{
				saveJson();
			}
		}
		
		std::shared_ptr<IMachineCache::MachineCacheEntry_t> MachineCacheJsonFile::GetEntry(std::string NamespaceURI)
		{
			auto it = m_cache.find(NamespaceURI);
			if (it != m_cache.end())
			{
				return std::make_shared<IMachineCache::MachineCacheEntry_t> (it->second);
			}

			return std::shared_ptr<MachineCacheEntry_t>();
		}

		void MachineCacheJsonFile::readJson()
		{
			std::ifstream i(m_cacheFilename);
			nlohmann::json j;
			i >> j;
			if(j.is_null())
			{
				return;
			}

			if (!j.is_array())
			{
				LOG(ERROR) << "MachineCache json must contain an array.";
				throw Util::Exception::ConfigurationException("MachineCache json must contain an array.");
			}

			for (auto el : j)
			{
				IMachineCache::MachineCacheEntry_t entry;

				if (!el.is_object())
				{
					std::stringstream ss;
					ss << "MachineCache Array element must be an object, got:" << std::setw(4) << j;
					LOG(ERROR) << ss.str();
					throw Util::Exception::ConfigurationException(ss.str().c_str());
				}

				auto jsonNamespaceUri = getValueOrException(el, JsonKey_Uri);
				if (!jsonNamespaceUri.is_string())
				{
					std::stringstream ss;
					ss << "Key '" << JsonKey_Uri << "' is not of type " << "string" << "in " << el << std::endl;
					LOG(ERROR) << ss.str();
					throw Util::Exception::ConfigurationException(ss.str().c_str());
				}
				entry.NamespaceUri = jsonNamespaceUri.get<std::string>();

				auto jsonTopicPrefix = getValueOrException(el, JsonKey_TopicPrefix);
				if (!jsonTopicPrefix.is_string())
				{
					std::stringstream ss;
					ss << "Key '" << JsonKey_Uri << "' is not of type " << "string" << "in " << el << std::endl;
					LOG(ERROR) << ss.str();
					throw Util::Exception::ConfigurationException(ss.str().c_str());
				}
				entry.TopicPrefix = jsonTopicPrefix.get<std::string>();

				m_cache.insert(std::make_pair(entry.NamespaceUri, entry));
			}
		}

		void MachineCacheJsonFile::saveJson()
		{
			nlohmann::json cacheJson;
			cacheJson = nlohmann::json::array();

			for (auto entry : m_cache)
			{
				nlohmann::json entryJson;
				entryJson[JsonKey_Uri] = entry.second.NamespaceUri;
				entryJson[JsonKey_TopicPrefix] = entry.second.TopicPrefix;

				cacheJson.push_back(entryJson);
			}

			std::ofstream o(m_cacheFilename);
			if (!o)
			{
				LOG(ERROR) << "Could not write ot file: " << m_cacheFilename;
				return;
			}

			o << std::setw(4) << cacheJson;

		}

	}
}
