/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 * Copyright 2023 (c) Sebastian Fried, FVA GmbH
 */

#include "ModelNodeIdToUaNodeId.hpp"

#include <stdexcept>
#include <iostream>
#include <easylogging++.h>

namespace Umati {
namespace OpcUa {
namespace Converter {


// inverse method of the hexmap used in open62541
char hexmapValue(char hex) {
    if(hex >= '0' && hex <= '9')
        return hex - '0';
    else if(hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    else if(hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;
    else
	    LOG(ERROR) << "Invalid char in Guid String";
        throw std::invalid_argument("Invalid char in Guid String");
 
}

//Convert an hex string to a UA_GUID
UA_Guid hex_to_UA_Guid(const char* hex) {
	UA_Guid out;
    size_t i = 0, j = 28;
    out.data1 = 0;
    for(; i<8;i++,j-=4)         /* pos 0-7, 4byte, (a) */
        out.data1 |= (UA_UInt32)(hexmapValue(hex[i]) & 0x0Fu) << j;
    i++;                        /* pos 8 */
    out.data2 = 0;
    for(j=12; i<13;i++,j-=4)    /* pos 9-12, 2byte, (b) */
        out.data2 |= (UA_UInt16)(hexmapValue(hex[i]) & 0x0Fu) << j;
    i++;                        /* pos 13 */
    out.data3 = 0;
    for(j=12; i<18;i++,j-=4)    /* pos 14-17, 2byte (c) */
        out.data3 |= (UA_UInt16)(hexmapValue(hex[i]) & 0x0Fu) << j;
    i++;                        /* pos 18 */
    memset(out.data4, 0, sizeof(out.data4));
    for(j=0;i<23;i+=2,j++) {     /* pos 19-22, 2byte (d) */
        out.data4[j] |= (UA_Byte)(hexmapValue(hex[i]) & 0x0Fu) << 4u;
        out.data4[j] |= (UA_Byte)(hexmapValue(hex[i+1]) & 0x0Fu);
    }
    i++;                        /* pos 23 */
    for(j=2; i<36;i+=2,j++) {    /* pos 24-35, 6byte (e) */
        out.data4[j] |= (UA_Byte)(hexmapValue(hex[i]) & 0x0Fu) << 4u;
        out.data4[j] |= (UA_Byte)(hexmapValue(hex[i+1]) & 0x0Fu);
    }
    return out;
}



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
	  guid = hex_to_UA_Guid(subs.c_str());
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