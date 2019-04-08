
#pragma once

#include <stdexcept>
#include "UmatiException.hpp"


namespace Umati
{
	namespace Exceptions {
		class ClientNotConnected : public UmatiException {
		public:
			using UmatiException::UmatiException;
		};
	}
}
