
#include "OpcUaNonGoodStatusCodeException.hpp"
#include <sstream>
#include <iomanip>

namespace Umati {
	namespace Exceptions {
		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(const UA_StatusCode&status,
																		 const std::string &message)
				: OpcUaException(toHex(status) + ", " + statusToMessage(status) + ": " + message) {

		}

		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(const UA_StatusCode &status)
				: OpcUaException(toHex(status) + ", " + statusToMessage(status)) {

		}

		std::string OpcUaNonGoodStatusCodeException::statusToMessage(const UA_StatusCode &status) {
			return std::string(UA_StatusCode_name(status));
		}

		std::string OpcUaNonGoodStatusCodeException::toHex(const UA_StatusCode &status, int digits) {
			std::stringstream ss;
			ss << std::setw(digits) << std::setfill('0') << std::hex << status;
			return ss.str();
		}
	}
}
