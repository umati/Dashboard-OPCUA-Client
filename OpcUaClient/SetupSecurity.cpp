
#include "SetupSecurity.hpp"

#include <string>

#include <fstream>
#include <uapkirsakeypair.h>
#include <uapkiidentity.h>
#include <uapkicertificate.h>
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
            if (directory.empty()) {
                return true;
            }

            std::vector <std::string> parts;

            std::string part = "";
            int i = 0;

            // Directory must end with a '/', otherwise last part is ignored (expected file)
            while (i < directory.size()) {
                if (directory[i] != '/') {
                    part += directory[i];
                } else {
                    parts.push_back(part);
                    part = "";
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

                if (mkdir(ss.str().c_str(), 0x777) != 0) {
                    if (errno != EEXIST) {
                        return false;
                    }
                }

            }

            return true;
        }

        bool SetupSecurity::setupSecurity(UaClientSdk::SessionSecurityInfo *sessionSecurityInfo) {
            {
                std::ifstream f(paths.ClientPrivCert.c_str());
                if (!f.good()) {
                    createDirs(paths.ServerRevokedCerts);
                    createDirs(paths.ServerTrustedCerts);
                    createDirs(paths.IssuerRevokedCerts);
                    createDirs(paths.IssuerTrustedCerts);

                    // File could not be opend, create it
                    if (!createNewClientCert()) {
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

            if (status.isBad()) {
                LOG(ERROR) << "initializePkiProviderOpenSSL failed. " << std::endl;
                return false;
            }

            status = sessionSecurityInfo->loadClientCertificateOpenSSL(
                    UaString(paths.ClientPubCert.c_str()),
                    UaString(paths.ClientPrivCert.c_str())
            );

            if (status.isBad()) {
                LOG(ERROR) << "initializePkiProviderOpenSSL failed. " << std::endl;
                return false;
            }

            return true;
        }

        bool SetupSecurity::createNewClientCert() {
            UaPkiRsaKeyPair keyPair(2048);
            UaPkiIdentity identity;

            identity.commonName = "OPCUA-DatenClient";
            identity.organization = "Created by ISW";
            identity.organizationUnit = "ISW";
            identity.locality = "LocationName";
            identity.state = "BW";
            identity.country = "DE";
            identity.domainComponent = "MyComputer";

            UaPkiCertificateInfo info;
            info.URI = "http://dashboard.umati.app/OPCUA_DataClient";
            info.DNSNames.create(1);
            UaString("example.com").copyTo(&info.DNSNames[0]);
            info.validTime = 3600 * 24 * 365 * 5; // seconds

            // create a self signed certificate
            UaPkiCertificate cert(info, identity, keyPair, false, UaPkiCertificate::SignatureAlgorithm_Sha256);
            if (cert.isNull()) {
                LOG(ERROR) << "cert is null" << std::endl;
                return false;
            }
            ///\todo check return types

            // encoded to DER format
            auto retCreateDer = cert.toDERFile(paths.ClientPubCert.c_str());

            if (!std::ifstream(paths.ClientPubCert.c_str()).good()) {
                LOG(ERROR) << "der-File creation failed: " << retCreateDer << std::endl;
                return false;
            }
            auto retCreatePEM = keyPair.toPEMFile(paths.ClientPrivCert.c_str(), 0);
            if (!std::ifstream(paths.ClientPubCert.c_str()).good()) {
                LOG(ERROR) << "pem-File creation failed: " << retCreatePEM << std::endl;
                return false;
            }

            return true;
        }

    }
}
