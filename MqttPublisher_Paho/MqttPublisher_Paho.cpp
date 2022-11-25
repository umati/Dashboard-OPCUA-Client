/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include "MqttPublisher_Paho.hpp"

#include <easylogging++.h>
#include <sstream>

namespace Umati {
	namespace MqttPublisher_Paho {

		MqttPublisher_Paho::MqttPublisher_Paho(const std::string &protocol, const std::string &host, std::uint16_t port, const std::string &username,
											   const std::string &password)
				: m_cli(getUri(protocol, host, port), getClientId(), 0, nullptr), m_callbacks(this) {
			m_cli.set_callback(m_callbacks);

	 		mqtt::connect_options opts_conn = getOptions(username, password);

			if (protocol == "wss") {
				mqtt::ssl_options ssl_opts;
#ifndef WIN32
				ssl_opts.ca_path("/etc/ssl/certs/");
#else
				ssl_opts.set_ca_path("./certs");
				ssl_opts.set_trust_store("./certs/ca-certificates.crt");
#endif
				ssl_opts.set_verify(true);
				opts_conn.set_ssl(ssl_opts);
			}
		
			mqtt::will_options opts_will = getLastWill();
			opts_conn.set_will(opts_will);

			try {
				LOG(INFO) << "Connect to " << host;
				m_cli.connect(opts_conn)->wait();
			}
			catch (const mqtt::exception &ex) {
				LOG(ERROR) << "Paho Exception:" << ex.what();
			}
		}

		std::string MqttPublisher_Paho::getUri(std::string protocol, std::string host, std::uint16_t port)
		{
			std::stringstream ss;
			ss << protocol << "://" << host << ":" << port << "/ws";
			return ss.str();
		}

		mqtt::connect_options MqttPublisher_Paho::getOptions(const std::string &username, const std::string &password) {
			mqtt::connect_options opts_conn;
			opts_conn.set_keep_alive_interval(std::chrono::seconds(10));
			opts_conn.set_clean_session(true);

			opts_conn.set_automatic_reconnect(2, 10);

			if (!username.empty()) {
				opts_conn.set_user_name(username);
			}

			if (!password.empty()) {
				opts_conn.set_password(password);
			}
			return opts_conn;
		}

		mqtt::will_options MqttPublisher_Paho::getLastWill() const {
			mqtt::will_options opts_will;
			opts_will.set_topic(m_onlineTopic);
			opts_will.set_payload(std::string("0"));
			opts_will.set_retained(true);
			return opts_will;
		}

		void MqttPublisher_Paho::Publish(std::string channel, std::string message) {
			try {
				m_cli.publish(channel, message, 0, true);
			}
			catch (const mqtt::exception &ex) {
				LOG(ERROR) << "Paho Exception:" << ex.what();
			}
		}

		std::string MqttPublisher_Paho::getClientId() {
			std::stringstream ss;
			ss << "Dashboard Paho Client ";

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis('a', 'z');
			for (int i = 0; i < 12; ++i) {
				ss << static_cast<char>(dis(gen));
			}
			return ss.str();
		}

		MqttPublisher_Paho::~MqttPublisher_Paho() {
			Publish(m_onlineTopic, "0");
			m_cli.disconnect();
		}

		MqttPublisher_Paho::MqttCallbacks::MqttCallbacks(MqttPublisher_Paho *mqttPublisher_paho) : m_mqttPublisher_paho(
				mqttPublisher_paho) {}

		void MqttPublisher_Paho::MqttCallbacks::connected(const std::string &cause) {
			LOG(INFO) << "Mqtt Connected: " << cause;
			m_mqttPublisher_paho->Publish(m_mqttPublisher_paho->m_onlineTopic, "1");
		}

		void MqttPublisher_Paho::MqttCallbacks::connection_lost(const std::string &cause) {
			LOG(ERROR) << "Connection lost: " << cause;
		}
	}
}
