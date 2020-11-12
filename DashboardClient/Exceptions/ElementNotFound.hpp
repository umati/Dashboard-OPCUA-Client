#pragma once

#include <stdexcept>
#include "UmatiException.hpp"

namespace Umati {
	namespace Exceptions {
		class ElementNotFound : public UmatiException {
		public:
			using UmatiException::UmatiException;

		};
	}
}
