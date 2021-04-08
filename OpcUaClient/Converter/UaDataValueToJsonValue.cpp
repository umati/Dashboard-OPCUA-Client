#include "UaDataValueToJsonValue.hpp"

#include <easylogging++.h>

namespace Umati {
	namespace OpcUa {
		namespace Converter {
			UaDataValueToJsonValue::UaDataValueToJsonValue(const UA_DataValue &dataValue,
														   bool serializeStatusInformation) {
				setValueFromDataValue(dataValue, serializeStatusInformation);
				if (serializeStatusInformation) {
					setStatusCodeFromDataValue(dataValue);
				}
			}

			void UaDataValueToJsonValue::setValueFromDataValue(const UA_DataValue &dataValue,
															   bool serializeStatusInformation) {
				auto &jsonValue = m_value;
				if (serializeStatusInformation) {
					jsonValue = m_value["value"];
				}
				
				auto variant = UA_Variant(dataValue.value);
				//VERIFY variant.type() == OpcUaType_Null
				if (UA_Variant_isEmpty(&variant)) {
					return;
				}
				if (!UA_Variant_isScalar(&variant)) {
					LOG(ERROR) << "Only scalar values are supported.";
				}

				switch (variant.type->typeKind) {
					/*
					//This has been checked before
					case OpcUaType_Null: {
						break;
					}*/

					//VERIFY UA_DATATYPEKIND_BOOLEAN or UA_TYPES_BOOLEAN
					case UA_DATATYPEKIND_BOOLEAN: {
						UA_Boolean v(variant.data);
						jsonValue = static_cast<bool>(v);
						break;
					}

					case UA_DATATYPEKIND_SBYTE: {
						UA_SByte v(*(UA_SByte*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_BYTE: {
						UA_Byte v(*(UA_Byte*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT16: {
						UA_Int16 v(*(UA_Int16*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT16: {
						UA_UInt16 v(*(UA_UInt16*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT32: {
						UA_Int32 v(*(UA_Int32*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT32: {
						UA_UInt32 v(*(UA_UInt32*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT64: {
						UA_Int64 v((UA_Int64)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT64: {
						UA_UInt64 v((UA_UInt64)variant.data);
						jsonValue = v;
						break;
					}
	
					
					case UA_DATATYPEKIND_FLOAT: {
						UA_Float v(*(UA_Float*)variant.data);
						jsonValue = v;
						break;
					}
					
					case UA_DATATYPEKIND_DOUBLE: {
						UA_Double v(*(UA_Double*)variant.data);
						jsonValue = v;
						break;
					}
					case UA_DATATYPEKIND_STRING: {
						UA_String s(*(UA_String*)variant.data);
						jsonValue = std::string((char*)s.data,s.length);
						break;
					}
				
					case UA_DATATYPEKIND_DATETIME: {
						UA_DateTime dateTime = ((UA_DateTime)variant.data);
						jsonValue = dateTime;
						break;
					}

					case UA_DATATYPEKIND_GUID: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_Guid. ";
						break;
					}

					case UA_DATATYPEKIND_BYTESTRING: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_ByteString. ";
						break;
					}

					case UA_DATATYPEKIND_XMLELEMENT: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_XmlElement. ";
						break;
					}
					case UA_DATATYPEKIND_NODEID: {
						UA_NodeId nodeId;
						UA_NodeId_init(&nodeId);
						nodeId = *(UA_NodeId*)variant.data;
						jsonValue = nodeId.identifier.numeric;
						break;
					}

					case UA_DATATYPEKIND_EXPANDEDNODEID: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_ExpandedNodeId. ";
						break;
					}

					case UA_DATATYPEKIND_STATUSCODE: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_StatusCode. ";
						break;
					}

					case UA_DATATYPEKIND_QUALIFIEDNAME: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_QualifiedName. ";
						break;
					}
					case UA_DATATYPEKIND_LOCALIZEDTEXT: {
						UA_LocalizedText localText(*(UA_LocalizedText*)variant.data);
						char *loc = "en-US";
						localText.locale.length = strlen(loc);
						localText.locale.data = (UA_Byte*)loc;
						jsonValue = {};
						//FIXME local is empty, leading to SEGV. Setting "en-US" manually.
						jsonValue["locale"] = std::string((char*)localText.locale.data,localText.locale.length);
						jsonValue["text"] =  std::string((char*)localText.text.data,localText.text.length);
						break;
					}
					
					case UA_DATATYPEKIND_EXTENSIONOBJECT: {
						UA_ExtensionObject exObj(*(UA_ExtensionObject*)variant.data);
						jsonValue = {};
						if (exObj.content.encoded.typeId.namespaceIndex != 0) {
							LOG(ERROR)
									<< "Not implemented conversion from OpcUaType_ExtensionObject with custom structured data type.";
							break;
						}

						switch (exObj.content.encoded.typeId.identifierType) {
							case UA_TYPES_RANGE: {
								UA_Range range(*(UA_Range*)exObj.content.decoded.data);
								jsonValue["low"] = range.low;
								jsonValue["high"] = range.high;
								break;
							}
							case UA_TYPES_EUINFORMATION: {
								UA_EUInformation euInfo(*(UA_EUInformation*)exObj.content.decoded.data);
								jsonValue["namespaceUri"] = std::string((char*)euInfo.namespaceUri.data,euInfo.namespaceUri.length);
								jsonValue["unitId"] = euInfo.unitId;
								
								{
									//VERIFY text = displayName and description = locale?
									UA_DataValue dataVal(*(UA_DataValue*)euInfo.displayName.text.data);
									jsonValue["displayName"] = UaDataValueToJsonValue(
										dataVal,
										serializeStatusInformation)
										.getValue();
								}
								{
									UA_DataValue dataVal(*(UA_DataValue*)euInfo.displayName.locale.data);
									jsonValue["description"] = UaDataValueToJsonValue(
										dataVal,
										serializeStatusInformation)
										.getValue();
								}
								break;
							}

							default: {
								LOG(ERROR) << "Not implemented conversion from type: "
										   << exObj.content.encoded.body.data;							}
						}
						break;
					}

					case UA_DATATYPEKIND_DATAVALUE: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_DataValue. ";
						break;
					}

					case UA_DATATYPEKIND_VARIANT: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_Variant. ";
						break;
					}

					case UA_DATATYPEKIND_DIAGNOSTICINFO: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_DiagnosticInfo. ";
						break;
					}
					default: {
						LOG(ERROR) << "Unknown data type. ";
						break;
					}
				}
			
			}
		
			void UaDataValueToJsonValue::setStatusCodeFromDataValue(const UA_DataValue &dataValue) {
				auto &jsonStatusCode = m_value["statusCode"];

				jsonStatusCode["code"] = dataValue.status;
				jsonStatusCode["isGood"] = ( dataValue.status == UA_STATUSCODE_GOOD) ?  UA_TRUE : UA_FALSE;
				
			}
		}
	}
}
