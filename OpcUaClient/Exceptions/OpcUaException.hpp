#pragma once

#include <stdexcept>
#include "UmatiException.hpp"
#include <statuscode.h>

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
