 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "ModelToUaConverter.hpp"
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			ModelToUaConverter::ModelToUaConverter(const std::map<std::string, uint16_t> &uriToID) : m_uriToID(
					uriToID) {
			}

			ModelToUaConverter::~ModelToUaConverter() = default;

			uint16_t ModelToUaConverter::getNsIndexFromUri(const std::string &uri) {
				if (uri.empty()) {
					return 0;
				}

				auto it = m_uriToID.find(uri);
				if (it == m_uriToID.end()) {
					LOG(ERROR) << "Could not find uri: " << uri << std::endl;
					return 0;
				}

				return it->second;
			}

		}
	}
}
