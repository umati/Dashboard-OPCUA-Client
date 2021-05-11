
#include "SetupSecurity.hpp"

// #include <python3.8/Python.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <easylogging++.h>
#include <unistd.h>


namespace Umati {
	namespace OpcUa {
		SetupSecurity::paths_t SetupSecurity::paths = {
				"./pki/",
				"./pki/server/trusted/",
				"./pki/server/revoked/",
				"./pki/client_cert.der",
				"./pki/client_key.der",
				"./pki/issuer/trusted/",
				"./pki/issuer/revoked/"
		};

		/* loadFile parses the certificate file.
		*
		* @param  path specifies the file name given in argv[]
		* @return Returns the file content after parsing */
		static UA_INLINE UA_ByteString
		loadFile(const char *const path)
		{
			UA_ByteString fileContents = UA_STRING_NULL;

			/* Open the file */
			FILE *fp = fopen(path, "rb");
			if (!fp)
			{
				errno = 0; /* We read errno also from the tcp layer... */
				return fileContents;
			}

			/* Get the file length, allocate the data and read */
			fseek(fp, 0, SEEK_END);
			fileContents.length = (size_t)ftell(fp);
			fileContents.data = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
			if (fileContents.data)
			{
				fseek(fp, 0, SEEK_SET);
				size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
				if (read != fileContents.length)
					UA_ByteString_clear(&fileContents);
			}
			else
			{
				fileContents.length = 0;
			}
			fclose(fp);

			return fileContents;
		}
		static bool createDirs(std::string directory) {
			if (directory.empty()) {
				return true;
			}

			std::vector<std::string> parts;

			std::string pathPart = "";
			uint i = 0;

			// Directory must end with a '/', otherwise last part is ignored (expected file)
			while (i < directory.size()) {
				if (directory[i] != '/') {
					pathPart += directory[i];
				} else {
					parts.push_back(pathPart);
					pathPart = "";
				}
				++i;
			}

			std::stringstream ss;
			bool first = true;
			for (const auto &part : parts) {
				if (part.empty()) {
					continue;
				}

				if (!first) {
					ss << "/";
				} else {
					first = false;
				}
				ss << part;

				if (part == "." || part == "..") {
					continue;
				}

				if (mkdir(ss.str().c_str(), mode_t(0777)) != 0) {
					if (errno != EEXIST) {
						return false;
					}
				}

			}

			return true;
		}
		bool SetupSecurity::setupSecurity(UA_ClientConfig *config, UA_Client *client) {
			std::ifstream f(paths.ClientPrivCert.c_str());
				if (!f.good()) {
					createDirs(paths.ServerRevokedCerts);
					createDirs(paths.ServerTrustedCerts);
					createDirs(paths.IssuerRevokedCerts);
					createDirs(paths.IssuerTrustedCerts);
				}
			
			createNewClientCert(); 

			UA_ByteString certificate = loadFile(paths.ClientPubCert.c_str());
    		UA_ByteString privateKey  = loadFile(paths.ClientPrivCert.c_str());

			//VERIFY do we need the trustlist?
			size_t trustListSize = 0;
			UA_ByteString *trustList = NULL;

			UA_ByteString *revocationList = NULL;
			size_t revocationListSize = 0;

			config->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
			UA_ClientConfig_setDefaultEncryption(config, certificate, privateKey,
												 trustList, trustListSize,
												 revocationList, revocationListSize);

			return true;
		}
		//TODO use python-dev C lib to execute the script instaed of system(), dont use Hardcoded path
		void SetupSecurity::createNewClientCert() {
				std::string command = "./Tools/create_self-signed.py -u urn:open62541.server.application -k 2048 -c client " + paths.PkiRoot;
				int retVal = system(command.c_str());
				if (retVal != 0){
					LOG(INFO) << "Creating for certs failed";
				}
			return;
		}

	}
}

