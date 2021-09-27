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
#include <Open62541Cpp/UA_NodeId.hpp>
#include <string>
#include <map>
#include <ModelOpcUa/ModelDefinition.hpp>

#include "UaToModelConverter.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			class UaNodeIdToModelNodeId : public UaToModelConverter {
			public:
				UaNodeIdToModelNodeId(open62541Cpp::UA_NodeId nodeId, const std::map<uint16_t, std::string> &idToUri);

				ModelOpcUa::NodeId_t getNodeId() {
					return m_nodeId;
				};

			private:

				ModelOpcUa::NodeId_t m_nodeId;

			};
		}
	}
}
