
#include "SetupSecurity.hpp"

// #include <python3.8/Python.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <easylogging++.h>
#include <sstream>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif


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
			unsigned int i = 0;

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
#if defined(_WIN32)
				if (_mkdir(ss.str().c_str()) != 0)
#else
				if (mkdir(ss.str().c_str(), mode_t(0x777)) != 0)
#endif
				{
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

			UA_ByteString certificate = loadFile(paths.ClientPubCert.c_str());
			UA_ByteString privateKey  = loadFile(paths.ClientPrivCert.c_str());
			if(certificate.length == 0 || privateKey.length == 0)
			{
				createNewClientCert();
				certificate = loadFile(paths.ClientPubCert.c_str());
				privateKey  = loadFile(paths.ClientPrivCert.c_str());
				if(certificate.length == 0 || privateKey.length == 0)
				{
					LOG(ERROR) << "Could not load client keyfiles ('" << paths.ClientPubCert << "', '" << paths.ClientPrivCert << "')";
				}
			}

			//VERIFY do we need the trustlist?
			size_t trustListSize = 0;
			UA_ByteString *trustList = NULL;

			UA_ByteString *revocationList = NULL;
			size_t revocationListSize = 0;

			UA_ClientConfig_setDefaultEncryption(config, certificate, privateKey,
												 trustList, trustListSize,
												 revocationList, revocationListSize);

			UA_ByteString_clear(&certificate);
			UA_ByteString_clear(&privateKey);

			return true;
		}
		//TODO use python-dev C lib to execute the script instaed of system(), dont use Hardcoded path
		void SetupSecurity::createNewClientCert() {
			std::stringstream command;
			#if defined(_WIN32)
				command << "python ";
			#else
				command << "python3 ";
			#endif
			command << "./Tools/create_self-signed.py -u urn:open62541.client.application -k 2048 -c client " << paths.PkiRoot;
			int retVal = system(command.str().c_str());
			if (retVal != 0){
				LOG(INFO) << "Creating for certs failed";
			}
			return;
		}

	}
}

