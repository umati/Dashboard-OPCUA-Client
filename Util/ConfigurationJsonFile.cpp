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
			parseConfigurationFile(j);
		}

		Configuration::MqttConfig ConfigurationJsonFile::Mqtt() {
			return m_mqtt;
		}

		Configuration::OpcUaConfig ConfigurationJsonFile::OpcUa() {
			return m_opcUa;
		}

		nlohmann::json ConfigurationJsonFile::getValueOrException(nlohmann::json json, std::string key) {
			auto it = json.find(key);
			if (it == json.end()) {
				std::stringstream ss;
				ss << "Key '" << key << "' not found." << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			return *it;
		}

		void ConfigurationJsonFile::parseConfigurationFile(nlohmann::json json) {
			auto jsonOpcUa = getValueOrException(json, JsonKey_OpcUa);
			if (!jsonOpcUa.is_object()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUa << "' is not of type " << "object" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			parseConfigurationOpcUa(jsonOpcUa);


			if (json.find(JsonKey_ObjectTypeNamespacesVector) != json.end()) {
				auto jsonObjectTypeNamespacesVector = getValueOrException(json, JsonKey_ObjectTypeNamespacesVector);
				m_ObjectTypeNamespacesVector = jsonObjectTypeNamespacesVector.get<std::vector<std::string>>();
			} else {
				LOG(INFO) << "No key " << JsonKey_ObjectTypeNamespacesVector << " found in configuration file";
				m_ObjectTypeNamespacesVector = std::vector<std::string>();
			}


			auto jsonMqtt = getValueOrException(json, JsonKey_Mqtt);
			if (!jsonMqtt.is_object()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt << "' is not of type " << "object" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			parseConfigurationMqtt(jsonMqtt);
		}

		void ConfigurationJsonFile::parseConfigurationMqtt(nlohmann::json json) {
			auto jsonHostname = getValueOrException(json, JsonKey_Mqtt_Hostname);
			if (!jsonHostname.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Hostname << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Hostname = jsonHostname.get<std::string>();

			auto jsonPort = getValueOrException(json, JsonKey_Mqtt_Port);
			if (!jsonPort.is_number_unsigned()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Port << "' is not of type " << "number_unsigned" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Port = jsonPort.get<std::uint16_t>();

			auto jsonUsername = getValueOrException(json, JsonKey_Mqtt_Username);
			if (!jsonUsername.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Username << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Username = jsonUsername.get<std::string>();

			auto jsonPassword = getValueOrException(json, JsonKey_Mqtt_Password);
			if (!jsonPassword.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Password << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Password = jsonPassword.get<std::string>();

		}

		void ConfigurationJsonFile::parseConfigurationOpcUa(nlohmann::json json) {
			auto jsonEndpoint = getValueOrException(json, JsonKey_OpcUa_Endpoint);
			if (!jsonEndpoint.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUa_Endpoint << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_opcUa.Endpoint = jsonEndpoint.get<std::string>();

			auto jsonUsername = getValueOrException(json, JsonKey_OpcUa_User);
			if (!jsonUsername.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUa_User << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_opcUa.Username = jsonUsername.get<std::string>();

			auto jsonPasswort = getValueOrException(json, JsonKey_OpcUa_Password);
			if (!jsonPasswort.is_string()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUa_Password << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_opcUa.Password = jsonPasswort.get<std::string>();

			auto jsonSecurity = getValueOrException(json, JsonKey_OpcUa_Security);
			if (!jsonSecurity.is_number()) {
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUa_Security << "' is not of type " << "number, uint8" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_opcUa.Security = jsonSecurity.get<std::uint8_t>();
		}

		std::vector<std::string> ConfigurationJsonFile::ObjectTypeNamespacesVector() {
			return m_ObjectTypeNamespacesVector;
		}
	}
}
