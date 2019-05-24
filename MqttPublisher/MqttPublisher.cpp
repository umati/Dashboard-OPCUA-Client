
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
			//reconnect_delay_set(1, 10, true);

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
				LOG(ERROR) << "MQTT-Publish error: ("<< ret  <<")" << mosquitto_strerror(ret);

				switch (ret) {
				case MOSQ_ERR_ERRNO:
				{
					auto err = errno;
					char* errorText = NULL;

					FormatMessageA(
						// use system message tables to retrieve error text
						FORMAT_MESSAGE_FROM_SYSTEM
						// allocate buffer on local heap for error text
						| FORMAT_MESSAGE_ALLOCATE_BUFFER
						// Important! will fail otherwise, since we're not
						// (and CANNOT) pass insertion parameters
						| FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
						err,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&errorText,  // output
						0, // minimum size for output buffer
						NULL);
					LOG(ERROR) << "Windows Error("<< errno <<"):" << errorText;

					if (NULL != errorText)
					{
						// ... do something with the string `errorText` - log it, display it to the user, etc.
						// release memory allocated by FormatMessage()
						LocalFree(errorText);
						errorText = NULL;
					}
					break;
				}

				}
			}
		}

		void MqttPublisher::on_disconnect(int rc)
		{
			LOG(WARNING) << "Mqtt disconnected, try reconnecting";
			reconnect();
		}

		void MqttPublisher::on_log(int level, const char * str)
		{
			switch (level)
			{
			case MOSQ_LOG_DEBUG:
			{
				LOG(DEBUG) << "Mosquitto log DEBUG: "<< str;
				break;
			}
			case MOSQ_LOG_INFO:
			{
				LOG(INFO) << "Mosquitto log INFO: " << str;
				break;
			}
			case MOSQ_LOG_NOTICE:
			{
				LOG(WARNING) << "Mosquitto log NOTICE: " << str;
				break;
			}
			case MOSQ_LOG_WARNING:
			{
				LOG(WARNING) << "Mosquitto log WARNING: " << str;
				break;
			}
			case MOSQ_LOG_ERR:
			{
				LOG(ERROR) << "Mosquitto log ERROR: " << str;
				break;
			}
			default:
			{
				LOG(WARNING) << "Mosquitto log UNKNOWN: " << level << ":" << str;
			}
			}
		}

		void MqttPublisher::on_error()
		{
			LOG(ERROR) << "Mosquitto error";
		}
	}
}
