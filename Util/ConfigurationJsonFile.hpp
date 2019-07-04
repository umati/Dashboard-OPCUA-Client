#pragma once

#include "Configuration.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace Umati
{
	namespace Util
	{
		class ConfigurationJsonFile : public Configuration
		{
		public:
			ConfigurationJsonFile(std::string filename);

			// Inherit from Configuration
			std::string OpcUaEndpoint() override;
			virtual std::string MachineCacheFile() override;
			MqttConfig Mqtt() override;

			const std::string JsonKey_OpcUaEndpoint = std::string("OpcUaEndpoint");
			const std::string JsonKey_MachineCacheFile = std::string("MachineCacheFile");
			
			const std::string JsonKey_Mqtt = std::string("Mqtt");
			const std::string JsonKey_Mqtt_Hostname = std::string("Hostname");
			const std::string JsonKey_Mqtt_Port = std::string("Port");
			const std::string JsonKey_Mqtt_Username = std::string("Username");
			const std::string JsonKey_Mqtt_Password = std::string("Password");
			const std::string JsonKey_Mqtt_TopicPrefix = std::string("TopicPrefix");

		protected:
			nlohmann::json getValueOrException(nlohmann::json json, std::string key);
			ConfigurationJsonFile() = default;
			virtual void parseConfigurationFile(nlohmann::json json);
			virtual void parseConfigurationMqtt(nlohmann::json json);

			std::string m_opcUaEndpoint;
			std::string m_machineCacheFile;
			MqttConfig m_mqtt;

		};
	}
}
