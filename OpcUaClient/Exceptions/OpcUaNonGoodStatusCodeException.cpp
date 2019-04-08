
#include "OpcUaNonGoodStatusCodeException.hpp"
#include <sstream>
#include <iomanip>

namespace Umati
{
	namespace Exceptions
	{
		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(UaStatusCode status, std::string message)
			: OpcUaException(toHex(status) + ", " + statusToMessage(status) + ": " + message)
		{

		}

		OpcUaNonGoodStatusCodeException::OpcUaNonGoodStatusCodeException(UaStatusCode status)
			: OpcUaException(toHex(status) + ", " + statusToMessage(status))
		{

		}

		std::string OpcUaNonGoodStatusCodeException::statusToMessage(UaStatusCode status)
		{
			return std::string(status.toString().toUtf8());
		}

		std::string OpcUaNonGoodStatusCodeException::toHex(UaStatusCode status, int digits)
		{
			std::stringstream ss;
			ss << std::setw(digits) << std::setfill('0') << std::hex << status.code();
			return ss.str();
		}
	}
}
