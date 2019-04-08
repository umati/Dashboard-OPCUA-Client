
#include "NodeIdToModelNodeId.hpp"
#include <easylogging++.h>

namespace umati
{
	namespace OpcUa
	{
		namespace Converter {
			NodeIdToModelNodeId::NodeIdToModelNodeId(UaNodeId nodeId, const std::map<uint16_t, std::string>& idToUri)
			{
				m_nodeId.Uri = getUriFormNsIndex(nodeId.namespaceIndex(), idToUri);
				nodeId.setNamespaceIndex(0);
				m_nodeId.Id = nodeId.toXmlString().toUtf8();
			}

			std::string NodeIdToModelNodeId::getUriFormNsIndex(uint16_t nsIndex, const std::map<uint16_t, std::string>& idToUri)
			{
				if (nsIndex == 0)
				{
					return std::string();
				}

				auto it = idToUri.find(nsIndex);
				if (it == idToUri.end())
				{
					/// \todo errror handling
					LOG(ERROR) << "Could not find nsIndex: " << nsIndex << std::endl;
					return std::string();
				}

				return it->second;
			}
		}
	}
}
