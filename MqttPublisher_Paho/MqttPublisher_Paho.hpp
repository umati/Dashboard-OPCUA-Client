/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#pragma once

#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <IPublisher.hpp>
#include <mqtt/async_client.h>

namespace Umati {
	namespace MqttPublisher_Paho {
		class MqttPublisher_Paho : public Umati::Dashboard::IPublisher {
		public:
			MqttPublisher_Paho(
					const std::string &host,
					std::uint16_t port,
					const std::string &username = std::string(),
					const std::string &password = std::string()
			);

			virtual ~MqttPublisher_Paho();

			// Inherit from IPublisher
			void Publish(std::string channel, std::string message) override;

		private:
			static std::string getClientId();

			mqtt::will_options getLastWill() const;

			static mqtt::connect_options getOptions(const std::string &username, const std::string &password);
			static std::string getUri(std::string host, std::uint16_t port);

			class MqttCallbacks : public mqtt::callback {
				friend class MqttPublisher_Paho;
			public:
				MqttCallbacks(MqttPublisher_Paho *mqttPublisher_paho);

				void connected(const std::string &cause) override;

				void connection_lost(const std::string &cause) override;

				MqttPublisher_Paho *m_mqttPublisher_paho;
			};

			mqtt::async_client m_cli;
			MqttCallbacks m_callbacks;
			const std::string m_onlineTopic = "umati/opcUaToMqttOnline";
		};
	}
}
