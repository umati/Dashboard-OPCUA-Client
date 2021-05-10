#pragma once

#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <string>

namespace Umati {
	namespace OpcUa {
		class SetupSecurity {
		public:
			struct paths_t {
				std::string ServerTrustedCerts;
				std::string ServerRevokedCerts;

				std::string ClientPubCert;
				std::string ClientPrivCert;

				std::string IssuerTrustedCerts;
				std::string IssuerRevokedCerts;
			};

			static bool setupSecurity(UA_ClientConfig *config, UA_Client *client);

			static bool createNewClientCert();

		protected:
			static paths_t paths;
		};
	}
}
