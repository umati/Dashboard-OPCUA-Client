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
