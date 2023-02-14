/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2023 (c) Marc Fischer, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include <gtest/gtest.h>
#include <Configuration.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>
#include <MqttPublisher_Paho.hpp>
#include <ConfigureLogger.hpp>

TEST(OpcUaClient, CaCertificateLinux_SSLCerts) {
  // Test the CaCertPath pointing to /etc/ssl/certs
  Umati::Util::ConfigureLogger("DashboardOpcUaClient");
  Umati::Util::ConfigurationJsonFile conf("ConfigurationCa.json");

    EXPECT_NO_THROW(Umati::MqttPublisher_Paho::MqttPublisher_Paho publisher(
    conf.getMqtt().Protocol,
    conf.getMqtt().Hostname,
    conf.getMqtt().Port,
    conf.getMqtt().CaCertPath,
    conf.getMqtt().CaTrustStorePath,
    conf.getMqtt().Username,
    conf.getMqtt().Password));
}
TEST(OpcUaClient, CaCertificateLinux_CurlCerts) {
  // Test the CaTrustStorePath pointing to downloaded cacert.pem
  Umati::Util::ConfigureLogger("DashboardOpcUaClient");
  Umati::Util::ConfigurationJsonFile conf("ConfigurationCa2.json");

  EXPECT_NO_THROW(Umati::MqttPublisher_Paho::MqttPublisher_Paho publisher(
    conf.getMqtt().Protocol,
    conf.getMqtt().Hostname,
    conf.getMqtt().Port,
    conf.getMqtt().CaCertPath,
    conf.getMqtt().CaTrustStorePath,
    conf.getMqtt().Username,
    conf.getMqtt().Password));
}
