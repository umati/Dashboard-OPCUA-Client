#pragma once

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

			static bool setupSecurity();

			static bool createNewClientCert();

		protected:
			static paths_t paths;
		};
	}
}
