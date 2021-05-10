#include "ModelNodeIdToUaNodeId.hpp"

#include <iostream>
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
		enum NodeID_Type 
			{   NODEID_NUMERIC = (int)'i', 
				NODEID_STRING = (int)'s', 
				NODEID_GUID = (int)'g',
				NODEID_BYTESTRING = (int)'b'
			};


			ModelNodeIdToUaNodeId::ModelNodeIdToUaNodeId(const ModelOpcUa::NodeId_t &modelNodeId,
														 const std::map<std::string, uint16_t> &uriToID)
					: ModelToUaConverter(uriToID) {

				int type = (int)(modelNodeId.Id.substr(0,1)[0]);
				std::string subs = modelNodeId.Id.substr(2);	
				switch (type)
				{
				case NODEID_NUMERIC:
				    m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri),std::stoi(subs));
					break;

				case NODEID_STRING:					
					m_nodeId = open62541Cpp::UA_NodeId(getNsIndexFromUri(modelNodeId.Uri),subs);
					break;

				case NODEID_GUID:
					LOG(ERROR) << "Conversion to GUID not implemented.";
					throw "Conversion to GUID not implemented.";
					break;

				case NODEID_BYTESTRING:
					LOG(ERROR) << "Conversion to ByteString not implemented.";
					throw "Conversion to ByteString not implemented.";
					break;
				
				default:
					LOG(ERROR) << "Identifier type not valid";
					throw "Identifier type not valid";
					break;
				}

		}
	}	
}
}