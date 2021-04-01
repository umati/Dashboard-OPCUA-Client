
#include "SetupSecurity.hpp"

#include <string>

#include <fstream>

#include <iostream>
#include <easylogging++.h>
#include <unistd.h>


namespace Umati {
	namespace OpcUa {
		SetupSecurity::paths_t SetupSecurity::paths = {
				"./pki/server/trusted/",
				"./pki/server/revoked/",
				"./pki/clientPub.der",
				"./pki/clientpriv.pem",
				"./pki/issuer/trusted/",
				"./pki/issuer/revoked/"
		};

		static bool createDirs(std::string directory) {
			return true;
		}

		bool SetupSecurity::setupSecurity() {
			return true;
		}

		bool SetupSecurity::createNewClientCert() {
			return true;
		}

	}
}
