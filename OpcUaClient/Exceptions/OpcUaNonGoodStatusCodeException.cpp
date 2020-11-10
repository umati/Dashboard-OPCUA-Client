
#include "OpcUaNonGoodStatusCodeException.hpp"
#include <sstream>
#include <iomanip>

namespace Umati {
	namespace Exceptions {
		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(const UaStatus &status,
																		 const std::string &message)
				: OpcUaException(toHex(status) + ", " + statusToMessage(status) + ": " + message) {

		}

		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(const UaStatus &status)
				: OpcUaException(toHex(status) + ", " + statusToMessage(status)) {

		}

		std::string OpcUaNonGoodStatusCodeException::statusToMessage(const UaStatus &status) {
			return std::string(status.toString().toUtf8());
		}

		std::string OpcUaNonGoodStatusCodeException::toHex(const UaStatus &status, int digits) {
			std::stringstream ss;
			ss << std::setw(digits) << std::setfill('0') << std::hex << status.code();
			return ss.str();
		}
	}
}
