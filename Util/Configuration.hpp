#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Umati {
	namespace Util {
		class Configuration {
		public:
			virtual ~Configuration() = 0;

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
				std::string Password;

				/// 1 = None, 2 Sign, 3 = Sign&Encrypt
				std::uint8_t Security = 1;
			};

			virtual std::vector<std::string> ObjectTypeNamespacesVector() = 0;

			virtual MqttConfig Mqtt() = 0;

			virtual OpcUaConfig OpcUa() = 0;
		};
	}
}
