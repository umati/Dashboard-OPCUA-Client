#pragma once

#include <string>
#include <uaqualifiedname.h>
#include <map>
#include <ModelOpcUa/ModelDefinition.hpp>

#include "UaToModelConverter.hpp"

namespace Umati {
    namespace OpcUa {
        namespace Converter {

            class UaQualifiedNameToModelQualifiedName : public UaToModelConverter {
            public:
                UaQualifiedNameToModelQualifiedName(UaQualifiedName qualifiedName,
                                                    const std::map <uint16_t, std::string> &idToUri);

                ModelOpcUa::QualifiedName_t getQualifiedName() {
                    return m_qualifiedName;
                };

            private:

                ModelOpcUa::QualifiedName_t m_qualifiedName;

            };
        }
    }
}
