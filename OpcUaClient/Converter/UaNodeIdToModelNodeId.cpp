
#include "UaNodeIdToModelNodeId.hpp"

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			UaNodeIdToModelNodeId::UaNodeIdToModelNodeId(UaNodeId nodeId,
														 const std::map<uint16_t, std::string> &idToUri)
					: UaToModelConverter(idToUri) {
				m_nodeId.Uri = getUriFromNsIndex(nodeId.namespaceIndex());
				nodeId.setNamespaceIndex(0);
				m_nodeId.Id = nodeId.toXmlString().toUtf8();
			}
		}
	}
}
