/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2022 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "SetupSecurity.hpp"

// #include <python3.8/Python.h>
#include <open62541/architecture_definitions.h>
#include <open62541/plugin/create_certificate.h>
#include <open62541/plugin/log_stdout.h>
#include <Open62541Cpp/UA_String.hpp>
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
UA_StatusCode bypassVerify(void *verificationContext, const UA_ByteString *cert) { return UA_STATUSCODE_GOOD; }

SetupSecurity::paths_t SetupSecurity::paths = {
  "./pki/",
  "./pki/server/trusted/",
  "./pki/server/revoked/",
  "./pki/client_cert.der",
  "./pki/client_key.der",
  "./pki/issuer/trusted/",
  "./pki/issuer/revoked/"};

/* saveFile is used to save the certificate file.
 *
 * @param  path specifies the file name given
 * @return Returns 0 if fpfrintf succeeded and -1 if it failed*/
static UA_INLINE int saveFile(const char *data, const char *path) {
  FILE *fp = fopen(path, "w");
  int retVal = fprintf(fp, "%s", data);
  fclose(fp);
  if (retVal > 0) {
    return 0;
  }
  return -1;
}

/* loadFile parses the certificate file.
 *
 * @param  path specifies the file name given
 * @return Returns the file content after parsing */
static UA_INLINE UA_ByteString loadFile(const char *const path) {
  UA_ByteString fileContents = UA_STRING_NULL;

  /* Open the file */
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    errno = 0; /* We read errno also from the tcp layer... */
    return fileContents;
  }

  /* Get the file length, allocate the data and read */
  fseek(fp, 0, SEEK_END);
  fileContents.length = (size_t)ftell(fp);
  fileContents.data = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
  if (fileContents.data) {
    fseek(fp, 0, SEEK_SET);
    size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
    if (read != fileContents.length) UA_ByteString_clear(&fileContents);
  } else {
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
    if (mkdir(ss.str().c_str(), mode_t(0775)) != 0)
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
  UA_ByteString privateKey = loadFile(paths.ClientPrivCert.c_str());
  if (certificate.length == 0 || privateKey.length == 0) {
    createNewClientCert(config);
    certificate = loadFile(paths.ClientPubCert.c_str());
    privateKey = loadFile(paths.ClientPrivCert.c_str());
    if (certificate.length == 0 || privateKey.length == 0) {
      LOG(ERROR) << "Could not load client keyfiles ('" << paths.ClientPubCert << "', '" << paths.ClientPrivCert << "')";
    }
  }

  // VERIFY do we need the trustlist?
  size_t trustListSize = 0;
  UA_ByteString *trustList = NULL;

  UA_ByteString *revocationList = NULL;
  size_t revocationListSize = 0;

  UA_ClientConfig_setDefaultEncryption(config, certificate, privateKey, trustList, trustListSize, revocationList, revocationListSize);

  UA_ByteString_clear(&certificate);
  UA_ByteString_clear(&privateKey);

  return true;
}

void SetupSecurity::createNewClientCert(UA_ClientConfig *config) {
  LOG(INFO) << "Creating new client certificate";
  UA_String subject[3] = {UA_STRING_STATIC("C=DE"), UA_STRING_STATIC("O=SampleOrganization"), UA_STRING_STATIC("CN=UmatiDashboardClient@localhost")};

  UA_UInt32 lenSubject = 3;
  std::stringstream ssSubAltNameUri;
  open62541Cpp::UA_String uri(&config->clientDescription.applicationUri, false);
  ssSubAltNameUri << "URI:" << static_cast<std::string>(uri);

  open62541Cpp::UA_String altNameUri(ssSubAltNameUri.str());
  UA_String subjectAltName[2] = {UA_STRING_STATIC("DNS:localhost"), *altNameUri.String};
  UA_UInt32 lenSubjectAltName = 2;
  UA_ByteString certificate = UA_BYTESTRING_NULL;
  UA_ByteString privateKey = UA_BYTESTRING_NULL;

  UA_StatusCode statusCertGen =
    UA_CreateCertificate(UA_Log_Stdout, subject, lenSubject, subjectAltName, lenSubjectAltName, 2048, UA_CERTIFICATEFORMAT_PEM, &privateKey, &certificate);

  if (statusCertGen != UA_STATUSCODE_GOOD) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Generating Certificate failed: %s", UA_StatusCode_name(statusCertGen));
    return;
  }

  saveFile((char *)certificate.data, paths.ClientPubCert.c_str());
  saveFile((char *)privateKey.data, paths.ClientPrivCert.c_str());

  return;
}

}  // namespace OpcUa
}  // namespace Umati
