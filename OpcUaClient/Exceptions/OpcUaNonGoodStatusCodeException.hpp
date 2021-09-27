 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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
