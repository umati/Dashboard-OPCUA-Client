#include "ModelNodeIdToUaNodeId.hpp"

#include <iostream>
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			ModelNodeIdToUaNodeId::ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t &modelNodeId,
														 const std::map<std::string, uint16_t> &uriToID)
					: ModelToUaConverter(uriToID) {

				//FIXME type? always numeric? better way to get id?
				std::string subs = modelNodeId.Id.substr(2);
				m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri),std::stoi(subs));
			
			}
		}
	}	
}