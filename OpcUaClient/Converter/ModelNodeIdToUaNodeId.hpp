#pragma once

#include <string>
#include <uanodeid.h>
#include <ModelOpcUa/ModelDefinition.hpp>
#include "ModelToUaConverter.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class ModelNodeIdToUaNodeId : public ModelToUaConverter {
			public:
				ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t& modelNodeId,
									  const std::map<std::string, uint16_t> &uriToID);

				UaNodeId getNodeId() {
					return m_nodeId;
				};
			private:

				UaNodeId m_nodeId;
			};
		}
	}
}
