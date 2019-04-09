#include "UaNodeClassToModelNodeClass.hpp"

#include <easylogging++.h>
#include <Exceptions/UmatiException.hpp>

namespace Umati
{
	namespace OpcUa
	{
		namespace Converter
		{
			UaNodeClassToModelNodeClass::UaNodeClassToModelNodeClass(OpcUa_NodeClass nodeClass)
			{
				switch (nodeClass)
				{
					case OpcUa_NodeClass_Object:
						m_nodeClass = ModelOpcUa::NodeClass_t::Object;
						break;
					case OpcUa_NodeClass_Variable:
						m_nodeClass = ModelOpcUa::NodeClass_t::Variable;
						break;

					case OpcUa_NodeClass_Method:
					case OpcUa_NodeClass_ObjectType:
					case OpcUa_NodeClass_VariableType:
					case OpcUa_NodeClass_ReferenceType:
					case OpcUa_NodeClass_DataType:
					case OpcUa_NodeClass_View:
					default:
						LOG(ERROR) << "Incompatible NodeClass: " << nodeClass;
						throw Exceptions::UmatiException("Incompatible NodeClass");
					
				}
			}
		}
	}
}
