/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2023 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2023 (c) Sebastian Friedl, FVA GmbH (for umati and VDW e.V.)
 */

#include "ConfigurationJsonFile.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "easylogging++.h"
#include "Exceptions/ConfigurationException.hpp"
#include <sstream>

namespace Umati {
namespace Util {
ConfigurationJsonFile::ConfigurationJsonFile(const std::string &filename) {
  std::ifstream i(filename);
  if (!i) {
    std::stringstream ss;
    ss << "File '" << filename << "' not found.";
    throw Exception::ConfigurationException(ss.str().c_str());
  }
  nlohmann::json j;
  try {
    i >> j;
    from_json(j, *this);
    Verify();
  } catch (nlohmann::json_abi_v3_11_2::detail::parse_error &ex) {
    std::stringstream ss;
    ss << "Json is not valid: '" << ex.what();
    throw Exception::ConfigurationException(ss.str().c_str());
  }
}

MqttConfig ConfigurationJsonFile::getMqtt() { return Mqtt; }

OpcUaConfig ConfigurationJsonFile::getOpcUa() { return OpcUa; }

std::vector<NamespaceInformation> ConfigurationJsonFile::getNamespaceInformations() { return NamespaceInformations; }

std::vector<std::string> ConfigurationJsonFile::getObjectTypeNamespaces() { return ObjectTypeNamespaces; }

bool ConfigurationJsonFile::hasMachinesFilter() { return MachinesFilter.size() != 0; }

std::vector<ModelOpcUa::NodeId_t> ConfigurationJsonFile::getMachinesFilter() { return MachinesFilter; }
}  // namespace Util
}  // namespace Umati
