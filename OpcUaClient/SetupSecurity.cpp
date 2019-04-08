
#include "SetupSecurity.hpp"

#include <string>

#include <fstream>
#include <uapkirsakeypair.h>
#include <uapkiidentity.h>
#include <uapkicertificate.h>
#include <iostream>
#include <easylogging++.h>

namespace umati {
	namespace OpcUa {
		SetupSecurity::paths_t SetupSecurity::paths = {
			"pki/server/trusted/",
			"pki/server/revoked/",
			"pki/clientPub.der",
			"pki/clientpriv.pem",
			"pki/issuer/trusted/",
			"pki/issuer/revoked/"
		};


		bool SetupSecurity::setupSecurity(UaClientSdk::SessionSecurityInfo * sessionSecurityInfo)
		{
			{
				std::ifstream f(paths.ClientPrivCert.c_str());
				if (!f.good())
				{
					// File could not be opend, create it
					if (!createNewClientCert())
					{
						return false;
					}
				}
			}

			/*********************************************************************
			Initialize the PKI provider for OpenSSL
			**********************************************************************/
			auto status = sessionSecurityInfo->initializePkiProviderOpenSSL(
				UaString(paths.ServerRevokedCerts.c_str()),
				UaString(paths.ServerTrustedCerts.c_str()),
				UaString(paths.IssuerRevokedCerts.c_str()),
				UaString(paths.IssuerTrustedCerts.c_str())
			);

			if (status.isBad())
			{
				LOG(ERROR) << "initializePkiProviderOpenSSL failed. " << std::endl;
				return false;
			}

			status = sessionSecurityInfo->loadClientCertificateOpenSSL(
				UaString(paths.ClientPubCert.c_str()),
				UaString(paths.ClientPrivCert.c_str())
			);

			if (status.isBad())
			{
				LOG(ERROR) << "initializePkiProviderOpenSSL failed. " << std::endl;
				return false;
			}

			return true;
		}

		bool SetupSecurity::createNewClientCert()
		{
			UaPkiRsaKeyPair keyPair(2048);
			UaPkiIdentity   identity;

			identity.commonName = "OPC UA DatenClient";
			identity.organization = "Created by ISW";
			identity.organizationUnit = "ISW";
			identity.locality = "LocationName";
			identity.state = "BW";
			identity.country = "DE";
			identity.domainComponent = "MyComputer";

			UaPkiCertificateInfo info;
			info.URI = "http://koni.vdw.de/OPCUA_DataClient";
			info.DNSNames.create(1);
			UaString("example.com").copyTo(&info.DNSNames[0]);
			info.validTime = 3600 * 24 * 365 * 5; // seconds

			// create a self signed certificate
			UaPkiCertificate cert(info, identity, keyPair, false, UaPkiCertificate::SignatureAlgorithm_Sha256);
			if (cert.isNull())
			{
				LOG(ERROR) << "cert is null" << std::endl;
				return false;
			}
			///\todo check return types

			// encoded to DER format
			if (cert.toDERFile(paths.ClientPubCert.c_str()))
			{
				LOG(ERROR) << "der-File creation failed" << std::endl;
				return false;
			}

			if (keyPair.toPEMFile(paths.ClientPrivCert.c_str(), 0))
			{
				LOG(ERROR) << "pem-File creation failed" << std::endl;
				return false;
			}

			return true;
		}

	}
}
