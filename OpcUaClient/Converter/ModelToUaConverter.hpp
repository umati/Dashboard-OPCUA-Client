 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#pragma once

#include <Open62541Cpp/UA_NodeId.hpp>
#include <open62541/client.h>
#include <string>
#include <map>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class ModelToUaConverter {
			public:
				explicit ModelToUaConverter(const std::map<std::string, uint16_t> &uriToID);

				virtual ~ModelToUaConverter() = 0;

			protected:
				uint16_t getNsIndexFromUri(const std::string &uri);

				const std::map<std::string, uint16_t> &m_uriToID;
			};
		}
	}
}
