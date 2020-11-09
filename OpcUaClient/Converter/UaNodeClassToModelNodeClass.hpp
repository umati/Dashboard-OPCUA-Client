#pragma once

#include <string>
#include <uaclientsdk.h>

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			class UaNodeClassToModelNodeClass {
			public:
				UaNodeClassToModelNodeClass(OpcUa_NodeClass nodeClass);

				ModelOpcUa::NodeClass_t getNodeClass() {
					return m_nodeClass;
				};

			private:

				ModelOpcUa::NodeClass_t m_nodeClass;

			};
		}
	}
}
