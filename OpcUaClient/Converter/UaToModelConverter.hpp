 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#pragma once

#include <string>
#include <map>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class UaToModelConverter {
			public:
				explicit UaToModelConverter(const std::map<uint16_t, std::string> &idToUri);

				virtual ~UaToModelConverter() = 0;

			protected:

				std::string getUriFromNsIndex(uint16_t nsIndex);

				const std::map<uint16_t, std::string> &m_idToUri;
			};
		}
	}
}
