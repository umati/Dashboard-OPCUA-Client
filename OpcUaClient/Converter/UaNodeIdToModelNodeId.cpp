
#include "UaNodeIdToModelNodeId.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			UaNodeIdToModelNodeId::UaNodeIdToModelNodeId(open62541Cpp::UA_NodeId nodeId,
														 const std::map<uint16_t, std::string> &idToUri)
					: UaToModelConverter(idToUri) {
				m_nodeId.Uri = getUriFromNsIndex(nodeId.NodeId->namespaceIndex);
				nodeId.NodeId->namespaceIndex = 0;
				m_nodeId.Id = std::string(nodeId);;
			}
		}
	}
}
