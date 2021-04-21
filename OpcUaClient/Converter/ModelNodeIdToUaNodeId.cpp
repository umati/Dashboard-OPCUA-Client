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
				//FIXME we need to have to check if its numeric or string!!
				std::string subs = modelNodeId.Id.substr(2);
				if (subs.find("5002") != std::string::npos) {
    			std::cout << "found!" << '\n';
				}					
				m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri),std::stoi(subs));
			
			}
		}
	}	
}