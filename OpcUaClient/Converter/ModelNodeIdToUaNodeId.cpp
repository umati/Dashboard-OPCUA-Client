/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "ModelNodeIdToUaNodeId.hpp"

#include <stdexcept>
#include <iostream>
#include <easylogging++.h>

namespace Umati {
namespace OpcUa {
namespace Converter {
enum NodeID_Type { NODEID_NUMERIC = (int)'i', NODEID_STRING = (int)'s', NODEID_GUID = (int)'g', NODEID_BYTESTRING = (int)'b' };

ModelNodeIdToUaNodeId::ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t &modelNodeId, const std::map<std::string, uint16_t> &uriToID)
  : ModelToUaConverter(uriToID) {
  int type = (int)(modelNodeId.Id.substr(0, 1)[0]);
  std::string subs = modelNodeId.Id.substr(2);
  switch (type) {
    case NODEID_NUMERIC:
      m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri), std::stoi(subs));
      break;

    case NODEID_STRING:
      m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri), subs);
      break;

    case NODEID_GUID:
      UA_Guid guid;
      // Convert std::string to UA_Guid
      {
        int ret = sscanf(
          subs.c_str(),
          "{%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx}",
          &guid.data1,
          &guid.data2,
          &guid.data3,
          &guid.data4[0],
          &guid.data4[1],
          &guid.data4[2],
          &guid.data4[3],
          &guid.data4[4],
          &guid.data4[5],
          &guid.data4[6],
          &guid.data4[7]);
        if (ret != 11) {
          LOG(ERROR) << "Failed to convert string to UA_Guid";
        }
      }

      m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri), guid);
      break;

    case NODEID_BYTESTRING:
      LOG(ERROR) << "Conversion to ByteString not implemented.";
      throw std::invalid_argument("Conversion to ByteString not implemented.");
      break;
    default:
      LOG(ERROR) << "Identifier type not valid";
      throw "Identifier type not valid";
      break;
  }
}
}  // namespace Converter
}  // namespace OpcUa
}  // namespace Umati