/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include <gtest/gtest.h>
#include <Configuration.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>

TEST(ConfigurationJsonFile, NormalConfiguration) {
  Umati::Util::ConfigurationJsonFile conf("Configuration.json");
  EXPECT_EQ(conf.getOpcUa().Endpoint, "opc.tcp://localhost:4840");
  EXPECT_EQ(conf.getOpcUa().Username, "User");
  EXPECT_EQ(conf.getOpcUa().Password, "Password");
  EXPECT_EQ(conf.getOpcUa().Security, 1);
  std::vector<std::string> objectTypeNamespaces;
  objectTypeNamespaces.emplace_back("http://www.umati.info");
  EXPECT_EQ(conf.getObjectTypeNamespaces(), objectTypeNamespaces);
  EXPECT_EQ(conf.getMqtt().Hostname, "localhost");
  EXPECT_EQ(conf.getMqtt().Port, 1883);
  EXPECT_EQ(conf.getMqtt().Username, "MyUser");
  EXPECT_EQ(conf.getMqtt().Password, "MyPassword");
}

TEST(ConfigurationJsonFile, WithoutNamespaces) {
  Umati::Util::ConfigurationJsonFile conf("Configuration2.json");
  EXPECT_EQ(conf.getOpcUa().Endpoint, "opc.tcp://localhost:4840");
  EXPECT_EQ(conf.getOpcUa().Username, "User");
  EXPECT_EQ(conf.getOpcUa().Password, "Password");
  EXPECT_EQ(conf.getOpcUa().Security, 1);
  std::vector<std::string> objectTypeNamespaces;
  EXPECT_EQ(conf.getObjectTypeNamespaces(), objectTypeNamespaces);
  EXPECT_EQ(conf.getMqtt().Hostname, "localhost");
  EXPECT_EQ(conf.getMqtt().Port, 1883);
  EXPECT_EQ(conf.getMqtt().ClientId, "test/test");
  EXPECT_EQ(conf.getMqtt().Username, "MyUser");
  EXPECT_EQ(conf.getMqtt().Password, "MyPassword");
}

TEST(ConfigurationJsonFile, FileNotFound) {
  EXPECT_THROW(Umati::Util::ConfigurationJsonFile conf("ConfigurationNotThere.json"), Umati::Util::Exception::ConfigurationException);
}
