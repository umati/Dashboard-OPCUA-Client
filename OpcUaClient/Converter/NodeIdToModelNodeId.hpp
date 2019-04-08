#pragma once

#include <string>
#include <uanodeid.h>
#include <map>
#include <ModelOpcUa/ModelDefinition.hpp>

namespace umati
{
	namespace OpcUa
	{
		namespace Converter {

			class NodeIdToModelNodeId
			{
			public:
				NodeIdToModelNodeId(UaNodeId nodeId, const std::map<uint16_t, std::string> &idToUri);

				ModelOpcUa::NodeId_t getNodeId() {
					return m_nodeId;
				};

			private:
				std::string getUriFormNsIndex(uint16_t nsIndex, const std::map<uint16_t, std::string> &idToUri);

				ModelOpcUa::NodeId_t m_nodeId;

			};
		}
	}
}
