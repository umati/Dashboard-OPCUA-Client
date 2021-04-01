#pragma once

#include <string>

namespace Umati {
	namespace Dashboard {
		class IPublisher {
		public:
			virtual void Publish(std::string channel, std::string message) = 0;
		};
	}
}
