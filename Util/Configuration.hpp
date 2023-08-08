/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2023 (c) Marc Fischer, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include "../ModelOpcUa/src/ModelOpcUa/ModelDefinition.hpp"

namespace Umati {
namespace Util {
struct MqttConfig {
  ///  Hostname or IP-Address
  std::string Hostname;
  std::uint16_t Port = 0;
  /// Might be empty if no authentification required
  std::string Username;
  /// Might be empty if no authentification required
  std::string Password;
  std::string Prefix = "umati/v2";
  /// Must be provided
  std::string ClientId;
  std::string Protocol = "tcp";
#ifndef WIN32
  std::string CaCertPath = "/etc/ssl/certs/";
  std::string CaTrustStorePath = "";
#else
  std::string CaCertPath = "./certs";
  std::string CaTrustStorePath = "./certs/cacert.pem";
#endif
  std::string HttpProxy = "";
  std::string HttpsProxy = "";
};

struct OpcUaConfig {
  /// OPC UA Endpoint
  std::string Endpoint;
  std::string Username;
  std::string Password;
  /// 1 = None, 2 Sign, 3 = Sign&Encrypt
  std::uint8_t Security = 1;
  bool ByPassCertVerification = false;
};

/**
 * @brief NamespaceInformation
 * Describes how to handle types introduced by a namespace.
 */
struct NamespaceInformation {
  std::string Namespace;                   /**< Namespace, e.g. https://opcfoundation.org/SurfaceTechnology */
  std::vector<ModelOpcUa::NodeId_t> Types; /**< Types, this Namespace introduces */
  ModelOpcUa::NodeId_t IdentificationType; /**< IdentificationType, child of types */
};

class Configuration {
 public:
  virtual ~Configuration() = 0;

  virtual std::vector<NamespaceInformation> getNamespaceInformations() = 0;

  virtual std::vector<std::string> getObjectTypeNamespaces() = 0;

  virtual MqttConfig getMqtt() = 0;

  virtual OpcUaConfig getOpcUa() = 0;

  virtual bool hasMachinesFilter() = 0;

  virtual std::vector<ModelOpcUa::NodeId_t> getMachinesFilter() = 0;
  /// Verify, that there is a valid configuration. Throws an exception, if an error is detected.
  virtual void Verify();
};
}  // namespace Util
}  // namespace Umati
