#pragma once

#include <string>
#include <map>

namespace Umati {
    namespace OpcUa {
        namespace Converter {
            class ModelToUaConverter {
            public:
                ModelToUaConverter(const std::map <std::string, uint16_t> &uriToID);

                virtual ~ModelToUaConverter() = 0;

            protected:
                uint16_t getNsIndexFromUri(std::string uri);

                const std::map <std::string, uint16_t> &m_uriToID;
            };
        }
    }
}
