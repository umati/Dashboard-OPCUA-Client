#pragma once

#include "Configuration.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace ModelOpcUa
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NodeId_t, Uri, Id);
}
namespace Umati {
	namespace Util {
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MqttConfig, Hostname, Port, Username, Password);
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OpcUaConfig, Endpoint, Username, Password, Security);
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NamespaceInformation, Namespace, Types, IdentificationType);

		class ConfigurationJsonFile : public Configuration {
		public:
			explicit ConfigurationJsonFile(const std::string &filename);
			// Inherit from Configuration
			OpcUaConfig getOpcUa() override;
			MqttConfig getMqtt() override;
			std::vector<NamespaceInformation> getNamespaceInformations() override;
			std::vector<std::string> getObjectTypeNamespaces() override;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConfigurationJsonFile, OpcUa, ObjectTypeNamespaces, NamespaceInformations, Mqtt)
		protected:
			nlohmann::json getValueOrException(nlohmann::json json, std::string key);
			ConfigurationJsonFile() = default;
			OpcUaConfig OpcUa;
			std::vector<std::string> ObjectTypeNamespaces;
			std::vector<NamespaceInformation> NamespaceInformations;
			MqttConfig Mqtt;
		};
	}
}
