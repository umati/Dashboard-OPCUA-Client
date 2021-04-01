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
