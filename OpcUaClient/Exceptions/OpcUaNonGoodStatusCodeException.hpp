#pragma once
#include <open62541/client.h>
#include <stdexcept>
#include <Exceptions/OpcUaException.hpp>


namespace Umati {
	namespace Exceptions {
		class OpcUaNonGoodStatusCodeException : public OpcUaException {
		public:
			using OpcUaException::OpcUaException;

			OpcUaNonGoodStatusCodeException(const UA_StatusCode &status, const std::string &message);

			explicit OpcUaNonGoodStatusCodeException(const UA_StatusCode &status);

		protected:
			static std::string statusToMessage(const UA_StatusCode &status);

			static std::string toHex(const UA_StatusCode &status, int digits = 8);

		};
	}
}
