
#include <IPublisher.hpp>
#include <cpp_redis/cpp_redis>

namespace Umati {
	namespace RedisPublisher {
		class RedisPublisher : public Umati::Dashboard::IPublisher
		{
		public:
			RedisPublisher(std::string host, std::uint16_t port = 6379);

			// Inherit form IPublisher
			virtual void Publish(std::string channel, std::string message) override;

		private:
			cpp_redis::client m_client;
		};
	}
}
