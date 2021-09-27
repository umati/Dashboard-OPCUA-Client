 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "UaNodeClassToModelNodeClass.hpp"

#include <easylogging++.h>
#include <Exceptions/UmatiException.hpp>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			UaNodeClassToModelNodeClass::UaNodeClassToModelNodeClass(UA_NodeClass nodeClass) {
				switch (nodeClass) {
					case UA_NODECLASS_OBJECT:
						m_nodeClass = ModelOpcUa::NodeClass_t::Object;
						break;

					case UA_NODECLASS_VARIABLE:
						m_nodeClass = ModelOpcUa::NodeClass_t::Variable;
						break;

					case UA_NODECLASS_METHOD:
						m_nodeClass = ModelOpcUa::NodeClass_t::Method;
						break;

					case UA_NODECLASS_OBJECTTYPE:
						m_nodeClass = ModelOpcUa::NodeClass_t::ObjectType;
						break;

					case UA_NODECLASS_VARIABLETYPE:
						m_nodeClass = ModelOpcUa::NodeClass_t::VariableType;
						break;

					case UA_NODECLASS_REFERENCETYPE:
						m_nodeClass = ModelOpcUa::NodeClass_t::ReferenceType;
						break;

					case UA_NODECLASS_DATATYPE:
						m_nodeClass = ModelOpcUa::NodeClass_t::DataType;
						break;

					case UA_NODECLASS_VIEW:
					default:
						LOG(ERROR) << "Incompatible NodeClass: " << nodeClass;
						throw Exceptions::UmatiException("Incompatible NodeClass");

				}
			}
		}
	}
}
