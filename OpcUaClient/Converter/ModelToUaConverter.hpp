#pragma once

#include <Open62541Cpp/UA_NodeId.hpp>
#include <open62541/client.h>
#include <string>
#include <map>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class ModelToUaConverter {
			public:
				explicit ModelToUaConverter(const std::map<std::string, uint16_t> &uriToID);

				virtual ~ModelToUaConverter() = 0;

			protected:
				uint16_t getNsIndexFromUri(const std::string &uri);

				const std::map<std::string, uint16_t> &m_uriToID;
			};
		}
	}
}
