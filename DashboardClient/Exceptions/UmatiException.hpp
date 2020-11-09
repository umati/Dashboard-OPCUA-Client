#pragma once

#include <stdexcept>

namespace Umati {
	namespace Exceptions {
		class UmatiException : public std::runtime_error {
		public:
			using std::runtime_error::runtime_error;
		};
	}
}
