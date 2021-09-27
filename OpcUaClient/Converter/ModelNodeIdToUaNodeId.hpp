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
#include <ModelOpcUa/ModelDefinition.hpp>
#include "ModelToUaConverter.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class ModelNodeIdToUaNodeId : public ModelToUaConverter {
			public:
				ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t &modelNodeId,
									  const std::map<std::string, uint16_t> &uriToID);

				open62541Cpp::UA_NodeId getNodeId() {
					return m_nodeId;
				};
			private:

				open62541Cpp::UA_NodeId m_nodeId;
			};
		}
	}
}
