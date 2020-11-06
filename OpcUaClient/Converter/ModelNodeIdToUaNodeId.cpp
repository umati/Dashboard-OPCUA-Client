#include "ModelNodeIdToUaNodeId.hpp"
#include <list>
#include <sstream>

#include <iostream>
#include <easylogging++.h>

namespace Umati {
    namespace OpcUa {
        namespace Converter {
            ModelNodeIdToUaNodeId::ModelNodeIdToUaNodeId(ModelOpcUa::NodeId_t modelNodeId,
                                                         const std::map <std::string, uint16_t> &uriToID)
                    : ModelToUaConverter(uriToID) {
                m_nodeId = UaNodeId::fromXmlString(UaString(modelNodeId.Id.c_str()));

                auto nsIndex = getNsIndexFromUri(modelNodeId.Uri);
                m_nodeId.setNamespaceIndex(nsIndex);
            }
        }
    }
}