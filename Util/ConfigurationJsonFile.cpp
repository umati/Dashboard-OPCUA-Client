#include "ConfigurationJsonFile.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "easylogging++.h"
#include "Exceptions/ConfigurationException.hpp"
#include <sstream>

namespace Umati {
	namespace Util {
		ConfigurationJsonFile::ConfigurationJsonFile(const std::string &filename) {
			std::ifstream i(filename);
			if (!i) {
				std::stringstream ss;
				ss << "File '" << filename << "' not found.";
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			nlohmann::json j;
			i >> j;
			from_json(j, *this);
		}

		MqttConfig ConfigurationJsonFile::getMqtt() {
			return Mqtt;
		}

		OpcUaConfig ConfigurationJsonFile::getOpcUa() {
			return OpcUa;
		}

		std::vector<NamespaceInformation> ConfigurationJsonFile::getNamespaceInformations() {
			return NamespaceInformations;
		}

		std::vector<std::string> ConfigurationJsonFile::getObjectTypeNamespaces() {
			return ObjectTypeNamespaces;
		}
	}
}
