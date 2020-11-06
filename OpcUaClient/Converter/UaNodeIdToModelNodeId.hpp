#pragma once

#include <string>
#include <uanodeid.h>
#include <map>
#include <ModelOpcUa/ModelDefinition.hpp>

#include "UaToModelConverter.hpp"

namespace Umati {
    namespace OpcUa {
        namespace Converter {

            class UaNodeIdToModelNodeId : public UaToModelConverter {
            public:
                UaNodeIdToModelNodeId(UaNodeId nodeId, const std::map <uint16_t, std::string> &idToUri);

                ModelOpcUa::NodeId_t getNodeId() {
                    return m_nodeId;
                };

            private:

                ModelOpcUa::NodeId_t m_nodeId;

            };
        }
    }
}
