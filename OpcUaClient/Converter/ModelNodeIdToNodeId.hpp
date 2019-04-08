#pragma once

#include <string>
#include <uanodeid.h>
#include <map>
#include <ModelOpcUa/ModelDefinition.hpp>

namespace umati {
	namespace OpcUa
	{
		namespace Converter {
			class ModelNodeIdToNodeId
			{
			public:
				ModelNodeIdToNodeId(ModelOpcUa::NodeId_t modelNodeId, const std::map<std::string, uint16_t> &uriToID);

				UaNodeId getNodeId() {
					return m_nodeId;
				};
			private:
				uint16_t getNsIndexFormUri(std::string uri, const std::map<std::string, uint16_t> &uriToID);

				UaNodeId m_nodeId;
			};
		}
	}
}
