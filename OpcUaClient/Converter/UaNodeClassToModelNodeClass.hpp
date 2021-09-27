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
#include <open62541/client.h>
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			class UaNodeClassToModelNodeClass {
			public:
				explicit UaNodeClassToModelNodeClass(UA_NodeClass nodeClass);

				ModelOpcUa::NodeClass_t getNodeClass() {
					return m_nodeClass;
				};

			private:

				ModelOpcUa::NodeClass_t m_nodeClass;

			};
		}
	}
}
