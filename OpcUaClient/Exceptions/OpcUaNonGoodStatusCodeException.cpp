 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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
