#include "ConfigurationJsonFile.hpp"
#include "ConfigurationJsonFile.hpp"
#include "ConfigurationJsonFile.hpp"
#include "ConfigurationJsonFile.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "Exceptions/ConfigurationException.hpp"
#include <sstream>

namespace Umati
{
	namespace Util
	{
		ConfigurationJsonFile::ConfigurationJsonFile(std::string filename)
		{
			std::ifstream i(filename);
			if (!i)
			{
				std::stringstream ss;
				ss << "File '" << filename << "' not found.";
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			nlohmann::json j;
			i >> j;
			parseConfigurationFile(j);
		}

		std::string ConfigurationJsonFile::OpcUaEndpoint()
		{
			return m_opcUaEndpoint;
		}
		std::string ConfigurationJsonFile::InstanceNamespaceURI()
		{
			return m_instanceNamespaceURI;
		}

		Configuration::MqttConfig ConfigurationJsonFile::Mqtt()
		{
			return m_mqtt;
		}

		nlohmann::json ConfigurationJsonFile::getValueOrException(nlohmann::json json, std::string key)
		{
			auto it = json.find(key);
			if (it == json.end())
			{
				std::stringstream ss;
				ss << "Key '" << key << "' not found." << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			return *it;
		}

		void ConfigurationJsonFile::parseConfigurationFile(nlohmann::json json)
		{
			auto jsonOpcUaEndpoint = getValueOrException(json, JsonKey_OpcUaEndpoint);
			if (!jsonOpcUaEndpoint.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_OpcUaEndpoint << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			m_opcUaEndpoint = jsonOpcUaEndpoint.get<std::string>();

			auto jsonInstanceNamespaceURI = getValueOrException(json, JsonKey_InstanceNamespaceURI);
			if (!jsonInstanceNamespaceURI.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_InstanceNamespaceURI << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			m_instanceNamespaceURI = jsonInstanceNamespaceURI.get<std::string>();

			auto jsonMqtt = getValueOrException(json, JsonKey_Mqtt);
			if (!jsonMqtt.is_object())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt << "' is not of type " << "object" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}
			parseConfigurationMqtt(jsonMqtt);
		}

		void ConfigurationJsonFile::parseConfigurationMqtt(nlohmann::json json)
		{
			auto jsonHostname = getValueOrException(json, JsonKey_Mqtt_Hostname);
			if (!jsonHostname.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Hostname << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Hostname = jsonHostname.get<std::string>();

			auto jsonPort = getValueOrException(json, JsonKey_Mqtt_Port);
			if (!jsonPort.is_number_unsigned())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Port << "' is not of type " << "number_unsigned" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Port = jsonPort.get<std::uint16_t>();

			auto jsonUsername = getValueOrException(json, JsonKey_Mqtt_Username);
			if (!jsonUsername.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Username << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Username = jsonUsername.get<std::string>();

			auto jsonPassword = getValueOrException(json, JsonKey_Mqtt_Password);
			if (!jsonPassword.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_Password << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.Password = jsonPassword.get<std::string>();

			auto jsonTopicPrefix = getValueOrException(json, JsonKey_Mqtt_TopicPrefix);
			if (!jsonTopicPrefix.is_string())
			{
				std::stringstream ss;
				ss << "Key '" << JsonKey_Mqtt_TopicPrefix << "' is not of type " << "string" << std::endl;
				throw Exception::ConfigurationException(ss.str().c_str());
			}

			m_mqtt.TopicPrefix = jsonTopicPrefix.get<std::string>();

		}
	}
}
