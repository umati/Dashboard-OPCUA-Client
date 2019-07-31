#include "UaDataValueToJsonValue.hpp"

#include <uadatetime.h>
#include <uarange.h>
#include <easylogging++.h>

namespace Umati
{
	namespace OpcUa
	{
		namespace Converter
		{
			UaDataValueToJsonValue::UaDataValueToJsonValue(const UaDataValue &dataValue)
			{
				setValueFromDataValue(dataValue);

			}

			void UaDataValueToJsonValue::setValueFromDataValue(const UaDataValue & dataValue)
			{
				auto &jsonValue = m_value["value"];
				jsonValue = nullptr;

				auto variant = UaVariant(UaVariant::clone(*(dataValue.value())), OpcUa_True);
				if (variant.type() == OpcUaType_Null)
				{
					return;
				}

				if (variant.arrayType() != OpcUa_VariantArrayType_Scalar)
				{
					LOG(ERROR) << "Only scalar values are supported.";
				}

				switch (variant.type())
				{
				case OpcUaType_Null:
				{
					jsonValue = nullptr;
					break;
				}

				case OpcUaType_Boolean:
				{
					OpcUa_Boolean v;
					variant.toBool(v);
					jsonValue = static_cast<bool>(v);
					break;
				}

				case OpcUaType_SByte:
				{
					OpcUa_SByte v;
					variant.toSByte(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Byte:
				{
					OpcUa_Byte v;
					variant.toByte(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Int16:
				{
					OpcUa_Int16 v;
					variant.toInt16(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_UInt16:
				{
					OpcUa_UInt16 v;
					variant.toUInt16(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Int32:
				{
					OpcUa_Int32 v;
					variant.toInt32(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_UInt32:
				{
					OpcUa_UInt32 v;
					variant.toUInt32(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Int64:
				{
					OpcUa_Int64 v;
					variant.toInt64(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_UInt64:
				{
					OpcUa_UInt64 v;
					variant.toUInt64(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Float:
				{
					OpcUa_Float v;
					variant.toFloat(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_Double:
				{
					OpcUa_Double v;
					variant.toDouble(v);
					jsonValue = v;
					break;
				}

				case OpcUaType_String:
				{
					jsonValue = variant.toString().toUtf8();
					break;
				}

				case OpcUaType_DateTime:
				{
					UaDateTime dateTime;
					variant.toDateTime(dateTime);
					jsonValue = dateTime.toString().toUtf8();

					break;
				}

				case OpcUaType_Guid:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_Guid. ";
					break;
				}

				case OpcUaType_ByteString:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_ByteString. ";
					break;
				}

				case OpcUaType_XmlElement:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_XmlElement. ";
					break;
				}

				case OpcUaType_NodeId:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_NodeId. ";
					break;
				}

				case OpcUaType_ExpandedNodeId:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_ExpandedNodeId. ";
					break;
				}

				case OpcUaType_StatusCode:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_StatusCode. ";
					break;
				}

				case OpcUaType_QualifiedName:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_QualifiedName. ";
					break;
				}

				case OpcUaType_LocalizedText:
				{
					UaLocalizedText localText;
					variant.toLocalizedText(localText);
					jsonValue = {};
					jsonValue["locale"] = UaString(localText.locale()).toUtf8();
					jsonValue["text"] = UaString(localText.text()).toUtf8();
					break;
				}

				case OpcUaType_ExtensionObject:
				{
					UaExtensionObject exObj;
					variant.toExtensionObject(exObj);
					jsonValue = {};

					if (exObj.dataTypeId().namespaceIndex() != 0)
					{
						LOG(ERROR) << "Not implemented conversion from OpcUaType_ExtensionObject with custom structured data type.";
						break;
					}

					switch (exObj.dataTypeId().identifierNumeric())
					{
					case OpcUaId_Range:
					{
						UaRange range(exObj);
						jsonValue["low"] = range.getLow();
						jsonValue["high"] = range.getHigh();
						break;
					}

					default:
					{
						LOG(ERROR) << "Not implemented conversion from type: " << exObj.dataTypeId().toFullString().toUtf8();
					}
					}
					break;
				}

				case OpcUaType_DataValue:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_DataValue. ";
					break;
				}

				case OpcUaType_Variant:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_Variant. ";
					break;
				}

				case OpcUaType_DiagnosticInfo:
				{
					LOG(ERROR) << "Not implemented conversion to OpcUaType_DiagnosticInfo. ";
					break;
				}
				default:
				{
					LOG(ERROR) << "Unknown data type. ";
					break;
				}
				}
			}
		}
	}
}
