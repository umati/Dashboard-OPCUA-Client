#include "MqttPublisher_Paho.hpp"

#include <easylogging++.h>

namespace Umati {
	namespace MqttPublisher_Paho {

		MqttPublisher_Paho::MqttPublisher_Paho(std::string host, std::uint16_t port, std::string username, std::string password)
			: m_cli(host, getClientId(), 100, nullptr), m_callbacks(this)
		{
			m_cli.set_callback(m_callbacks);
			mqtt::connect_options opts_conn;
			opts_conn.set_keep_alive_interval(std::chrono::seconds(10));
			opts_conn.set_clean_session(true);
			
			opts_conn.set_automatic_reconnect(2, 10);
			
			if (!username.empty())
			{
				opts_conn.set_user_name(username);
			}

			if (!password.empty())
			{
				opts_conn.set_password(password);
			}
			
			mqtt::will_options opts_will;
			opts_will.set_topic(OnlineTopic);
			opts_will.set_payload(std::string("0"));
			opts_will.set_retained(true);
			
			opts_conn.set_will(opts_will);
			try {
				LOG(ERROR) << "Connect to " << host;
				m_cli.connect(opts_conn)->wait();
			}
			catch (const mqtt::exception& ex)
			{
				LOG(ERROR) << "Paho Exception:" << ex.what();
			}
			
		}

		void MqttPublisher_Paho::Publish(std::string channel, std::string message)
		{
			try {
				m_cli.publish(channel, message, 0, true);
			}
			catch (const mqtt::exception& ex)
			{

				LOG(ERROR) << "Paho Exception:" << ex.what();
			}
			
		}

		std::string MqttPublisher_Paho::getClientId()
		{
			std::stringstream ss;
			ss << "Dashboard Paho Client ";

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis('a', 'z');
			for (int i = 0; i < 12; ++i)
			{
				ss << static_cast<char>(dis(gen));
			}
			return ss.str();
		}

		MqttPublisher_Paho::~MqttPublisher_Paho()
		{
			Publish(OnlineTopic, "0");
			m_cli.disconnect();
		}

		MqttPublisher_Paho::MqttCallbacks::MqttCallbacks(MqttPublisher_Paho * mqttPublisher_paho)
			: m_mqttPublisher_paho(mqttPublisher_paho)
		{
		}

		void MqttPublisher_Paho::MqttCallbacks::connected(const std::string & cause)
		{
			LOG(ERROR) << "Mqtt Connected: " << cause;
			m_mqttPublisher_paho->Publish(m_mqttPublisher_paho->OnlineTopic, "1");
		}

		void MqttPublisher_Paho::MqttCallbacks::connection_lost(const std::string & cause)
		{
			LOG(ERROR) << "Connection lost: " << cause;
		}
	}
}
