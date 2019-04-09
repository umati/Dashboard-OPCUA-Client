#pragma once

#include <stdexcept>
#include "UmatiException.hpp"

namespace Umati
{
	namespace Exceptions
	{
		class OpcUaException : public UmatiException {
		public:
			using UmatiException::UmatiException;

		};
	}
}
