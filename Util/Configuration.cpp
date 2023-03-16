/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2023 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include "Configuration.hpp"
#include "Exceptions/ConfigurationException.hpp"

namespace Umati {
namespace Util {
Configuration::~Configuration() = default;

void Configuration::Verify() {
  auto mqtt = this->getMqtt();
  if (mqtt.ClientId.empty()) {
    throw Exception::ConfigurationException("MQTT ClientId is not specified.");
  }
  if (mqtt.Hostname.empty()) {
    throw Exception::ConfigurationException("MQTT Hostname is not specified.");
  }
  if (mqtt.Port == 0) {
    throw Exception::ConfigurationException("MQTT Port is not specified.");
  }
  auto opcua = this->getOpcUa();
  if (opcua.Endpoint.empty()) {
    throw Exception::ConfigurationException("OPC UA endpoint is not specified.");
  }
}
}  // namespace Util
}  // namespace Umati
