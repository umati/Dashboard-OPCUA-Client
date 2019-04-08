#include "ModelNodeIdToNodeId.hpp"
#include <list>
#include <sstream>

#include <iostream>
#include <easylogging++.h>

namespace umati {
	namespace OpcUa
	{
		namespace Converter {
			ModelNodeIdToNodeId::ModelNodeIdToNodeId(ModelOpcUa::NodeId_t modelNodeId, const std::map<std::string, uint16_t> &uriToID)
			{
				m_nodeId = UaNodeId::fromXmlString(UaString(modelNodeId.Id.c_str()));
				
				auto nsIndex = getNsIndexFormUri(modelNodeId.Uri, uriToID);
				m_nodeId.setNamespaceIndex(nsIndex);
			}

			uint16_t Converter::ModelNodeIdToNodeId::getNsIndexFormUri(std::string uri, const std::map<std::string, uint16_t>& uriToID)
			{
				if (uri.empty())
				{
					return 0;
				}

				auto it = uriToID.find(uri);
				if (it == uriToID.end())
				{
					/// \todo errror handling
					LOG(ERROR) << "Could not find uri: " << uri << std::endl;
					return 0;
				}

				return it->second;
			}
		}
	}
}