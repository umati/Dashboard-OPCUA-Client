
#include "RedisPublisher.hpp"
#include "winsock_initializer.h"
#include <easylogging++.h>

namespace Umati {
	namespace RedisPublisher {
		winsock_initializer winsock_init;

		RedisPublisher::RedisPublisher(std::string host, std::uint16_t port)
		{
			m_client.connect(host, port, [](const std::string &host, std::size_t port, cpp_redis::connect_state status) {
				if (status == cpp_redis::connect_state::dropped) {
					LOG(ERROR) << "redis client disconnected from " << host << ":" << port << std::endl;
				}
				LOG(INFO) << "redis client state changed to: " << (int)status << " for " << host << ":" << port << std::endl;
			});
			m_client.sync_commit();
		}

		void RedisPublisher::Publish(std::string channel, std::string message)
		{
			m_client.publish(channel, message);
			m_client.sync_commit();
		}
	}
}
