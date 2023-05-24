/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2023 (c) Sebastian Friedl, FVA GmbH
 */

#include "ModelNodeIdToUaNodeId.hpp"

#include <stdexcept>
#include <iostream>
#include <easylogging++.h>
#include <Open62541Cpp/UA_String.hpp>

namespace Umati {
namespace OpcUa {
namespace Converter {

enum NodeID_Type { NODEID_NUMERIC = (int)'i', NODEID_STRING = (int)'s', NODEID_GUID = (int)'g', NODEID_BYTESTRING = (int)'b' };

ModelNodeIdToUaNodeId::ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t &modelNodeId, const std::map<std::string, uint16_t> &uriToID)
  : ModelToUaConverter(uriToID) {
  open62541Cpp::UA_String tmp(modelNodeId.Id);
  UA_NodeId_parse(m_nodeId.NodeId, *tmp.String);
  m_nodeId.NodeId->namespaceIndex = getNsIndexFromUri(modelNodeId.Uri);
}
}  // namespace Converter
}  // namespace OpcUa
}  // namespace Umati