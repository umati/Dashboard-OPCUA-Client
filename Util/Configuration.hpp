#pragma once

#include <string>
#include <stdint.h>

namespace Umati
{
	namespace Util
	{
		class Configuration
		{
		public:
			inline virtual ~Configuration() = 0 {};

			struct MqttConfig
			{
				///  Hostname or IP-Address
				std::string Hostname;

				std::uint16_t Port;
				
				/// Might be empty if no authentification required
				std::string Username;
				
				/// Might be empty if no authentification required
				std::string Password;
			};

			inline virtual std::string OpcUaEndpoint() = 0;

			inline virtual std::string MachineCacheFile() = 0;
			
			inline virtual MqttConfig Mqtt() = 0;

		};
	}
}
