#pragma once

#include <nlohmann/json.hpp>
#include <open62541/client.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			class UaDataValueToJsonValue {
			public:
				explicit UaDataValueToJsonValue(const UA_DataValue &dataValue, bool serializeStatusInformation = false);

				nlohmann::json getValue() {
					return m_value;
				};
			protected:

				void setValueFromDataValue(const UA_DataValue &dataValue, bool serializeStatusInformation = false);

				void setStatusCodeFromDataValue(const UA_DataValue &dataValue);

				nlohmann::json m_value;
			};
		}
	}
}
