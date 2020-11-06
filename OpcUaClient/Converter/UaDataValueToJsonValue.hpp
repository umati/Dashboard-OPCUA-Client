#pragma once

#include <nlohmann/json.hpp>

#include <uadatavalue.h>

namespace Umati {
    namespace OpcUa {
        namespace Converter {
            class UaDataValueToJsonValue {
            public:
                UaDataValueToJsonValue(const UaDataValue &dataValue, bool serializeStatusInformation = false);

                nlohmann::json getValue() {
                    return m_value;
                };
            protected:

                void setValueFromDataValue(const UaDataValue &dataValue, bool serializeStatusInformation = false);

                void setStatusCodeFromDataValue(const UaDataValue &dataValue);

                nlohmann::json m_value;
            };
        }
    }
}
