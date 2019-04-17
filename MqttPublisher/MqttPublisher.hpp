
#include <IPublisher.hpp>

#include <mosquitto/mosquittopp.h>

namespace Umati {
	namespace MqttPublisher {
		class MqttPublisher : public Umati::Dashboard::IPublisher, private mosqpp::mosquittopp
		{
		public:
			MqttPublisher(std::string host, std::uint16_t port, std::string onlineTopic);
			virtual ~MqttPublisher();

			// Inherit form IPublisher
			virtual void Publish(std::string channel, std::string message) override;

			const std::string OnlineTopic;
		};
	}
}
