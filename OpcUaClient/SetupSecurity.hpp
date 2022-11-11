/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2022 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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

UA_StatusCode bypassVerify(void *verificationContext, const UA_ByteString *cert);
class SetupSecurity {
 public:
  struct paths_t {
    std::string PkiRoot;

    std::string ServerTrustedCerts;
    std::string ServerRevokedCerts;

    std::string ClientPubCert;
    std::string ClientPrivCert;

    std::string IssuerTrustedCerts;
    std::string IssuerRevokedCerts;
  };

  static bool setupSecurity(UA_ClientConfig *config, UA_Client *client);

  static UA_StatusCode bypassVerify(void *verificationContext, const UA_ByteString *cert);

  static void createNewClientCert(UA_ClientConfig *config);

 protected:
  static paths_t paths;
};
}  // namespace OpcUa
}  // namespace Umati
