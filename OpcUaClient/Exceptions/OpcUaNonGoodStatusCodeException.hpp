#pragma once

#include <stdexcept>
#include <Exceptions/OpcUaException.hpp>
#include <statuscode.h>

namespace Umati
{
	namespace Exceptions
	{
		class OpcUaNonGoodStatusCodeException : public OpcUaException {
		public:
			using OpcUaException::OpcUaException;
			OpcUaNonGoodStatusCodeException(const UaStatus& status, std::string message);
			explicit OpcUaNonGoodStatusCodeException(const UaStatus& status);

		protected:
			static std::string statusToMessage(const UaStatus& status);
			static std::string toHex(const UaStatus& status, int digits = 8);

		};
	}
}
