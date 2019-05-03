
#include "MqttPublisher.hpp"
#include <easylogging++.h>

struct MosquittoInit
{
	MosquittoInit()
	{
		mosqpp::lib_init();
	}

	~MosquittoInit()
	{
		mosqpp::lib_cleanup();
	}
};

static MosquittoInit mosInit;

namespace Umati {
	namespace MqttPublisher {

		MqttPublisher::MqttPublisher(
			std::string host,
			std::uint16_t port,
			std::string onlineTopic,
			std::string username,
			std::string password
		)
			: mosqpp::mosquittopp("Dashboard Client"), OnlineTopic(onlineTopic)
		{
			reconnect_delay_set(1, 10, true);

			std::string zero("0");
			auto ret = will_set(onlineTopic.c_str(), zero.length(), zero.c_str(), 0, true);

			if (ret != MOSQ_ERR_SUCCESS)
			{
				LOG(ERROR) << "MQTT- error while set_will: " << ret << std::endl;
			}

			if (!username.empty() && !password.empty())
			{
				auto ret = username_pw_set(username.c_str(), password.c_str());

				if (ret != MOSQ_ERR_SUCCESS)
				{
					LOG(ERROR) << "MQTT- error while set_will: " << ret << std::endl;
				}
			}

			ret = connect(host.c_str(), port);
			if (ret != MOSQ_ERR_SUCCESS)
			{
				LOG(ERROR) << "MQTT-Connect error: " << ret << std::endl;
			}

			Publish(onlineTopic, "1");


			threaded_set(false);
			loop_start();
		}

		MqttPublisher::~MqttPublisher()
		{
			this->Publish(OnlineTopic, "0");
			loop_stop(true);
		}

		void MqttPublisher::Publish(std::string channel, std::string message)
		{
			int mid;
			auto ret = publish(&mid, channel.c_str(), message.size(), message.c_str(), 0, true);
			if (ret != MOSQ_ERR_SUCCESS)
			{
				LOG(ERROR) << "MQTT-Publish error: " << ret << std::endl;
			}
		}

		void MqttPublisher::on_disconnect(int rc)
		{
			LOG(WARNING) << "Mqtt disconnected, try reconnecting";
			reconnect();
		}
	}
}
