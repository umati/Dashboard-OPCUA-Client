/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2023 (c) Marc Fischer, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include <gtest/gtest.h>

#include <Converter/UaNodeIdToModelNodeId.hpp>
#include <Converter/ModelNodeIdToUaNodeId.hpp>
#include <Converter/ModelQualifiedNameToUaQualifiedName.hpp>

TEST(Converter, Number_NodeId) {
  ModelOpcUa::NodeId_t nodeId{"MyURI", "i=10"};

  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  auto uaNodeId = Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, uri2Id).getNodeId();
  auto convNodeId = Umati::OpcUa::Converter::UaNodeIdToModelNodeId(uaNodeId, id2Uri).getNodeId();

  EXPECT_EQ(nodeId, convNodeId);
}

TEST(Converter, String_NodeId) {
  ModelOpcUa::NodeId_t nodeId{"MyURI", "s=StringId"};

  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  auto uaNodeId = Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, uri2Id).getNodeId();
  auto convNodeId = Umati::OpcUa::Converter::UaNodeIdToModelNodeId(uaNodeId, id2Uri).getNodeId();

  EXPECT_EQ(nodeId, convNodeId);
}

TEST(Converter, GUID_NodeId) {
  ModelOpcUa::NodeId_t nodeId{"MyURI", "g=1b9be88d-9249-4cfb-8d08-9a7e32d0d01d"};

  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  auto uaNodeId = Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, uri2Id).getNodeId();
  auto convNodeId = Umati::OpcUa::Converter::UaNodeIdToModelNodeId(uaNodeId, id2Uri).getNodeId();

  EXPECT_EQ(nodeId, convNodeId);
}

TEST(Converter, GUID_NodeId2) {
  UA_Guid myGuid = UA_Guid_random();
  auto nodeid = open62541Cpp::UA_NodeId(2,myGuid);
  
  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  auto modelNodeId = Umati::OpcUa::Converter::UaNodeIdToModelNodeId(nodeid, id2Uri).getNodeId();
  auto convNodeId = Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(modelNodeId, uri2Id).getNodeId();

  EXPECT_EQ(nodeid, convNodeId);
}

TEST(Converter, INVALID_GUID_NodeId2) {
  ModelOpcUa::NodeId_t nodeId{"MyURI", "g=not a GUID"};

  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  EXPECT_THROW(Umati::OpcUa::Converter::ModelNodeIdToUaNodeId(nodeId, uri2Id),std::invalid_argument);
}

TEST(Converter, QualifiedName) {
  ModelOpcUa::QualifiedName_t qualName{"MyURI", "MyName"};

  std::map<std::string, uint16_t> uri2Id{{"MyURI", 2}};
  std::map<uint16_t, std::string> id2Uri{{2, "MyURI"}};

  auto modQualName = Umati::OpcUa::Converter::ModelQualifiedNameToUaQualifiedName(qualName, uri2Id);
  auto uaQualName = modQualName.getQualifiedName();

  EXPECT_EQ(uaQualName.namespaceIndex, 2);
  EXPECT_EQ(std::string((char*)uaQualName.name.data, uaQualName.name.length), qualName.Name);
}
