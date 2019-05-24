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
		class MqttPublisher_Paho : public Umati::Dashboard::IPublisher
		{
		public:
			MqttPublisher_Paho(
				std::string host,
				std::uint16_t port,
				std::string onlineTopic,
				std::string username = std::string(),
				std::string password = std::string()
			);
			virtual ~MqttPublisher_Paho();

			// Inherit from IPublisher
			void Publish(std::string channel, std::string message) override;

			const std::string OnlineTopic;

		private:
			class MqttCallbacks : public mqtt::callback
			{
				friend class MqttPublisher_Paho;
			public:
				MqttCallbacks(MqttPublisher_Paho *mqttPublisher_paho);
				void connected(const std::string& cause) override;
				void connection_lost(const std::string& cause) override;
				MqttPublisher_Paho * m_mqttPublisher_paho;
			};

			mqtt::async_client m_cli;
			MqttCallbacks m_callbacks;
		};
	}
}
