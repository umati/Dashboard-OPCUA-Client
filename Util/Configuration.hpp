#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "../ModelOpcUa/src/ModelOpcUa/ModelDefinition.hpp"

namespace Umati {
	namespace Util {
		struct MqttConfig {
			///  Hostname or IP-Address
			std::string Hostname;
			std::uint16_t Port;
			/// Might be empty if no authentification required
			std::string Username;
			/// Might be empty if no authentification required
			std::string Password;
		};

		struct OpcUaConfig {
			/// OPC UA Endpoint
			std::string Endpoint;
			std::string Username;
			std::string Password;			/// 1 = None, 2 Sign, 3 = Sign&Encrypt
			std::uint8_t Security = 1;
		};

		/**
		 * @brief NamespaceInformation
		 * Describes how to handle types introduced by a namespace.
		 */
		struct NamespaceInformation {
			std::string Namespace; /**< Namespace, e.g. https://opcfoundation.org/SurfaceTechnology */
			std::vector<ModelOpcUa::NodeId_t> Types; /**< Types, this Namespace introduces */ 
			ModelOpcUa::NodeId_t IdentificationType; /**< IdentificationType, child of types */
		};

		class Configuration {
		public:
			virtual ~Configuration() = 0;

			virtual std::vector<NamespaceInformation> getNamespaceInformations() = 0;

			virtual std::vector<std::string> getObjectTypeNamespaces() = 0;

			virtual MqttConfig getMqtt() = 0;

			virtual OpcUaConfig getOpcUa() = 0;
		};
	}
}
