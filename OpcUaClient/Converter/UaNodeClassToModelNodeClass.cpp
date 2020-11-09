#include "UaNodeClassToModelNodeClass.hpp"

#include <easylogging++.h>
#include <Exceptions/UmatiException.hpp>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			UaNodeClassToModelNodeClass::UaNodeClassToModelNodeClass(OpcUa_NodeClass nodeClass) {
				switch (nodeClass) {
					case OpcUa_NodeClass_Object:
						m_nodeClass = ModelOpcUa::NodeClass_t::Object;
						break;

					case OpcUa_NodeClass_Variable:
						m_nodeClass = ModelOpcUa::NodeClass_t::Variable;
						break;

					case OpcUa_NodeClass_Method:
						m_nodeClass = ModelOpcUa::NodeClass_t::Method;
						break;

					case OpcUa_NodeClass_ObjectType:
						m_nodeClass = ModelOpcUa::NodeClass_t::ObjectType;
						break;

					case OpcUa_NodeClass_VariableType:
						m_nodeClass = ModelOpcUa::NodeClass_t::VariableType;
						break;

					case OpcUa_NodeClass_ReferenceType:
						m_nodeClass = ModelOpcUa::NodeClass_t::ReferenceType;
						break;

					case OpcUa_NodeClass_DataType:
						m_nodeClass = ModelOpcUa::NodeClass_t::DataType;
						break;

					case OpcUa_NodeClass_View:
					default:
						LOG(ERROR) << "Incompatible NodeClass: " << nodeClass;
						throw Exceptions::UmatiException("Incompatible NodeClass");

				}
			}
		}
	}
}
