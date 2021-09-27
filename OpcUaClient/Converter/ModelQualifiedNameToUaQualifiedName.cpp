 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "ModelQualifiedNameToUaQualifiedName.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			ModelQualifiedNameToUaQualifiedName::ModelQualifiedNameToUaQualifiedName(
					const ModelOpcUa::QualifiedName_t &modelQualifiedName,
					const std::map<std::string, uint16_t> &uriToID
			) : ModelToUaConverter(uriToID) {
				m_qualifiedName = UA_QUALIFIEDNAME_ALLOC(getNsIndexFromUri(modelQualifiedName.Uri), modelQualifiedName.Name.c_str());
			}
		}
	}
}