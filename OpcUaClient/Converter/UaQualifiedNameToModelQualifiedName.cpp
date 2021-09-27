 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "UaNodeIdToModelNodeId.hpp"
#include <easylogging++.h>
#include "UaQualifiedNameToModelQualifiedName.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			UaQualifiedNameToModelQualifiedName::UaQualifiedNameToModelQualifiedName(
					const open62541Cpp::UA_QualifiedName &qualifiedName,
					const std::map<uint16_t, std::string> &idToUri)
					: UaToModelConverter(idToUri) {
				m_qualifiedName.Uri = getUriFromNsIndex(qualifiedName.QualifiedName->namespaceIndex);
				m_qualifiedName.Name = std::string((char *)qualifiedName.QualifiedName->name.data,qualifiedName.QualifiedName->name.length);
			}
		}
	}
}
