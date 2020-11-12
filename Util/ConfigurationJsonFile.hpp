#pragma once

#include "Configuration.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace Umati {
	namespace Util {
		class ConfigurationJsonFile : public Configuration {
		public:
			explicit ConfigurationJsonFile(const std::string &filename);

			// Inherit from Configuration
			OpcUaConfig OpcUa() override;

			MqttConfig Mqtt() override;

			std::vector<std::string> ObjectTypeNamespacesVector() override;

			const std::string JsonKey_OpcUa = std::string("OpcUa");
			const std::string JsonKey_OpcUa_Endpoint = std::string("Endpoint");
			const std::string JsonKey_OpcUa_User = std::string("User");
			const std::string JsonKey_OpcUa_Password = std::string("Password");
			const std::string JsonKey_OpcUa_Security = std::string("Security");

			const std::string JsonKey_ObjectTypeNamespacesVector = std::string("ObjectTypeNamespaces");

			const std::string JsonKey_Mqtt = std::string("Mqtt");
			const std::string JsonKey_Mqtt_Hostname = std::string("Hostname");
			const std::string JsonKey_Mqtt_Port = std::string("Port");
			const std::string JsonKey_Mqtt_Username = std::string("Username");
			const std::string JsonKey_Mqtt_Password = std::string("Password");

		protected:
			nlohmann::json getValueOrException(nlohmann::json json, std::string key);

			ConfigurationJsonFile() = default;

			virtual void parseConfigurationFile(nlohmann::json json);

			virtual void parseConfigurationMqtt(nlohmann::json json);

			virtual void parseConfigurationOpcUa(nlohmann::json json);

			OpcUaConfig m_opcUa;
			std::vector<std::string> m_ObjectTypeNamespacesVector;
			MqttConfig m_mqtt;

		};
	}
}
