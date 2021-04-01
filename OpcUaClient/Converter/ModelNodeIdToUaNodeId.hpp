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
