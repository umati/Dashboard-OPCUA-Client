 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "UaToModelConverter.hpp"
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			UaToModelConverter::UaToModelConverter(const std::map<uint16_t, std::string> &idToUri) : m_idToUri(
					idToUri) {
			}

			UaToModelConverter::~UaToModelConverter() = default;


			std::string UaToModelConverter::getUriFromNsIndex(uint16_t nsIndex) {
				/*if (nsIndex == 0) {
					return std::string();
				}*/
				
				auto it = m_idToUri.find(nsIndex);
				if (it == m_idToUri.end()) {
					LOG(ERROR) << "Could not find nsIndex: " << nsIndex << std::endl;
					return std::string();
				}

				return it->second;
			}
		}
	}
}
