 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2022 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "UaDataValueToJsonValue.hpp"

#include <easylogging++.h>
#include "../../deps/open62541/src/ua_types_encoding_json.h"

namespace Umati {
	namespace OpcUa {
		namespace Converter {

			UaDataValueToJsonValue::UaDataValueToJsonValue(const UA_DataValue &dataValue,
														   UA_Client *c,
														   UA_NodeId nid,
														   bool serializeStatusInformation) {
				m_pClient = c;
				nodeId = nid;
				setValueFromDataValue(dataValue, serializeStatusInformation);
				if (serializeStatusInformation) {
					setStatusCodeFromDataValue(dataValue);
				}
			}

			void UaDataValueToJsonValue::setValueFromScalarVariant(UA_Variant &variant, nlohmann::json *jsonValue, bool serializeStatusInformation) {
				switch (variant.type->typeKind) {

					case UA_DATATYPEKIND_BOOLEAN: {
						UA_Boolean v(*(UA_Boolean*)variant.data);
						*jsonValue = static_cast<bool>(v);
						break;
					}

					case UA_DATATYPEKIND_SBYTE: {
						UA_SByte v(*(UA_SByte*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_BYTE: {
						UA_Byte v(*(UA_Byte*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT16: {
						UA_Int16 v(*(UA_Int16*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT16: {
						UA_UInt16 v(*(UA_UInt16*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT32: {
						UA_Int32 v(*(UA_Int32*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT32: {
						UA_UInt32 v(*(UA_UInt32*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_INT64: {
						UA_Int64 v(*(UA_Int64*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT64: {
						UA_UInt64 v(*(UA_UInt64*)variant.data);
						*jsonValue = v;
						break;
					}


					case UA_DATATYPEKIND_FLOAT: {
						UA_Float v(*(UA_Float*)variant.data);
						*jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_DOUBLE: {
						UA_Double v(*(UA_Double*)variant.data);
						*jsonValue = v;
						break;
					}
					case UA_DATATYPEKIND_STRING: {
						UA_String s(*(UA_String*)variant.data);
						*jsonValue = std::string((char*)s.data,s.length);
						break;
					}

					case UA_DATATYPEKIND_DATETIME: {
						UA_DateTime dateTime(*(UA_DateTime*)variant.data);
						auto dtStruct = UA_DateTime_toStruct(dateTime);
						//No to String() function. Using stringstream to build the string in the right format.
						std::stringstream dateTimeString;
						dateTimeString << dtStruct.year << "-" << dtStruct.month << "-" << dtStruct.day << "T"
						<< dtStruct.hour << ":" << dtStruct.min << ":" << dtStruct.sec << ":" << dtStruct.milliSec << "Z";

						*jsonValue = dateTimeString.str();
						break;
					}

					case UA_DATATYPEKIND_GUID: {
						UA_Guid guid(*(UA_Guid*) variant.data);
						std::stringstream str;
						str << std::hex << (uint32_t) guid.data1 
							<< '-' << (uint16_t) guid.data2 
							<< '-' << (uint16_t) guid.data3
							<< '-' << *(uint32_t*) &guid.data4[0] 
							<< '-' << *(uint32_t*) &guid.data4[4];
						*jsonValue = str.str();
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
						*jsonValue = nodeId.identifier.numeric;
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

					case UA_DATATYPEKIND_DATAVALUE: {
						UA_DataValue d(*(UA_DataValue*)variant.data);
						*jsonValue = {};
						(*jsonValue)["hasServerPicoseconds"] = static_cast<bool>(d.hasServerPicoseconds);
						(*jsonValue)["serverPicoseconds"] = d.serverPicoseconds;
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

					case UA_DATATYPEKIND_QUALIFIEDNAME: {
						LOG(ERROR) << "Not implemented conversion to OpcUaType_QualifiedName. ";
						break;
					}

					case UA_DATATYPEKIND_LOCALIZEDTEXT: {
						UA_LocalizedText localText(*(UA_LocalizedText*)variant.data);
						*jsonValue = {};
						// FIX_BEGIN (FIX_3)
						// Inserted nullchecks
						// Added a max length because length determination on some Servers return invalid high length numbers 
						std::size_t maxLength = 4000;
						if(localText.locale.data != nullptr && localText.locale.length < maxLength) {
						(*jsonValue)["locale"] = std::string((char*)localText.locale.data,localText.locale.length);
						}
						if(localText.text.data != nullptr && localText.text.length < maxLength) {
							(*jsonValue)["text"] =  std::string((char*)localText.text.data,localText.text.length);
						}
						// FIX_END
						break;
					}

					case UA_DATATYPEKIND_EXTENSIONOBJECT: {
						UA_ExtensionObject exObj(*(UA_ExtensionObject*)variant.data);
						if(exObj.encoding == UA_ExtensionObjectEncoding::UA_EXTENSIONOBJECT_DECODED) {
							LOG(INFO) << "Known conversion from OpcUaType_ExtensionObject with DataTypeEncodingType: ns="
								<< exObj.content.encoded.typeId.namespaceIndex << "; i=" << exObj.content.encoded.typeId.identifier.numeric;
							

							void *data = exObj.content.decoded.data;
							for (size_t i = 0; i < exObj.content.decoded.type->membersSize; i++) {
								UA_DataValue dataVal;
								UA_DataValue_init(&dataVal);
								if (exObj.content.decoded.type->members[i].isArray) {
									size_t arraySize = *((size_t*) data);
									void **pointerToArrayPointer = (void**)((UA_Byte*) data + sizeof(size_t));
									void *pointerToArray = *(pointerToArrayPointer);
									
									if (arraySize > 0) {
										UA_Variant_setArray(&dataVal.value,  
															(UA_Byte*) pointerToArray + exObj.content.decoded.type->members[i].padding, 
															arraySize,
															exObj.content.decoded.type->members[i].memberType);
										(*jsonValue)[std::string(exObj.content.decoded.type->members[i].memberName)] = UaDataValueToJsonValue(
											dataVal,
											m_pClient,
											nodeId,
											serializeStatusInformation)
											.getValue();
									}
								} 
								else {
									void *dataPointer = (UA_Byte*) data + exObj.content.decoded.type->members[i].padding;
									UA_Variant_setScalar(&dataVal.value, 
										dataPointer, 
										exObj.content.decoded.type->members[i].memberType);
									auto json = UaDataValueToJsonValue(
										dataVal,
										m_pClient,
										nodeId,
										serializeStatusInformation)
										.getValue();
									if(!json.is_null()) {
										(*jsonValue)[std::string(exObj.content.decoded.type->members[i].memberName)] = json;
									}
								}
								if (exObj.content.decoded.type->members[i].isArray) {
									data = (UA_Byte*) data + sizeof(void*) + sizeof(size_t);
								} else if (exObj.content.decoded.type->members[i].isOptional) {
									data = (UA_Byte*) data + sizeof(void*);
								} else {
									data = (UA_Byte*) data + exObj.content.decoded.type->members[i].memberType->memSize;
								}
								data = (UA_Byte*) data + exObj.content.decoded.type->members[i].padding;
							}
						} else {
							LOG(ERROR) << "Unknow conversion from OpcUaType_ExtensionObject with DataTypeEncodingType: ns="
								<< exObj.content.encoded.typeId.namespaceIndex << "; i=" << exObj.content.encoded.typeId.identifier.numeric;
							switch(nodeId.identifierType) {
								case UA_NodeIdType::UA_NODEIDTYPE_STRING:
								LOG(ERROR) << "The erroc originates from ns=" << nodeId.namespaceIndex << "; s=" << std::string((char *)nodeId.identifier.string.data, nodeId.identifier.string.length);
								break;
								case UA_NodeIdType::UA_NODEIDTYPE_NUMERIC:
								LOG(ERROR) << "The erroc originates from ns=" << nodeId.namespaceIndex << "; s=" << nodeId.identifier.numeric;
							}

							UA_NodeId dataTypeNodeId;
							UA_NodeId_init(&dataTypeNodeId);
							UA_Client_readDataTypeAttribute(m_pClient, nodeId, &dataTypeNodeId);
							UA_Variant v;
							UA_Variant_init(&v);
							UA_Client_readValueAttribute(m_pClient, nodeId, &v);
							
							// auto dataType = UA_Client_findDataType(m_pClient, &dataTypeNodeId);
							auto dataType = v.type;
							auto clientConfig = UA_Client_getConfig(m_pClient);
							auto res = UA_decodeBinaryInternal(&exObj.content.encoded.body, NULL, exObj.content.decoded.data, dataType, clientConfig->customDataTypes);
							LOG(ERROR) << "Found the Datatype=" << dataType->typeName << "And the decoding was success?: " << res;
						}
						break;
					}

					default: {
						void *data = variant.data;
						for (size_t i = 0; i < variant.type->membersSize; i++) {
							UA_DataValue dataVal;
							UA_DataValue_init(&dataVal);

							{	
								if (variant.type->members[i].isArray) {
									size_t arraySize = *((size_t*) data);
									void **pointerToArrayPointer = (void**)((UA_Byte*) data + sizeof(size_t));
									void *pointerToArray = *(pointerToArrayPointer);
									
									if (arraySize > 0) {
										UA_Variant_setArray(&dataVal.value,  (UA_Byte*) pointerToArray + variant.type->members[i].padding, arraySize,variant.type->members[i].memberType);
										(*jsonValue)[std::string(variant.type->members[i].memberName)] = UaDataValueToJsonValue(
											dataVal,
											m_pClient,
											nodeId,
											serializeStatusInformation)
											.getValue();
									}
								} 
								else {
									void *dataPointer = (UA_Byte*) data + variant.type->members[i].padding;
									if(variant.type->members[i].isOptional) {
										void** pointerToPointer = (void**)((UA_Byte*) data +variant.type->members[i].padding);
										dataPointer = *(pointerToPointer);
									}
									UA_Variant_setScalar(&dataVal.value, dataPointer, variant.type->members[i].memberType);
									auto json = UaDataValueToJsonValue(
										dataVal,
										m_pClient,
										nodeId,
										serializeStatusInformation)
										.getValue();
									if(!json.is_null()) {
										(*jsonValue)[std::string(variant.type->members[i].memberName)] = json;
									}
								}
								if (variant.type->members[i].isArray) {
									data = (UA_Byte*) data + sizeof(void*) + sizeof(size_t);
								} else if (variant.type->members[i].isOptional) {
									data = (UA_Byte*) data + sizeof(void*);
								} else {
									data = (UA_Byte*) data + variant.type->members[i].memberType->memSize;
								}
								data = (UA_Byte*) data + variant.type->members[i].padding;
							}
						}
						break;
					}
				}

			}
		

			template<typename T>
			void UaDataValueToJsonValue::getValueFromDataValueArray(const UA_Variant *variant, UA_UInt32 dimensionNumber,
																	nlohmann::json *j, T *variantData, bool serializeStatusInformation) {
				if (dimensionNumber == variant->arrayDimensionsSize - 1) {
					for(UA_UInt32 i = 0U; i < variant->arrayDimensions[dimensionNumber]; i++) {												
						nlohmann::json jsonValue;
						UA_Variant var = {
							variant->type,  			/* The data type description */
							variant->storageType,
							0,           				/* The number of elements in the data array */
							(void*) (&variantData[i]), /* Points to the scalar or array data */
							variant->arrayDimensionsSize, /* The number of dimensions */
							NULL						/* Pointer to dimensionsArray */
						};
						setValueFromScalarVariant(var, &jsonValue, serializeStatusInformation);
						j->push_back(jsonValue);
					}
					return;
				}				
				UA_UInt32 offset = 1;
				for(UA_UInt32 i = dimensionNumber + 1; i < variant->arrayDimensionsSize - 1; i++) {
					offset = offset * variant->arrayDimensions[i];
				}
				for(UA_UInt32 i = 0; i < variant->arrayDimensions[dimensionNumber]; i++) {
					auto nestedj = nlohmann::json::array();
					getValueFromDataValueArray<T>(variant, dimensionNumber + 1, &nestedj, &variantData[i * offset], serializeStatusInformation);
					j->push_back(nestedj);
				}
			}


#define VALUEFROMDATAARRAY(ENUM, VAR) UA_##VAR *v = (UA_##VAR*) variant.data; \
		getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), jsonValue, v, serializeStatusInformation); \

#define SIMPLECASE(ENUM, VAR) case UA_DATATYPEKIND_##ENUM: { \
		UA_##VAR *v = (UA_##VAR*) variant.data; \
		getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), jsonValue, v, serializeStatusInformation); \
		break; }

#define CASENOTIMPLEMENTED(ENUM, VAR) case UA_DATATYPEKIND_##ENUM: { \
		LOG(ERROR) << "Not implented conversion to UA_" << #VAR; \
		break; }

			void UaDataValueToJsonValue::setValueFromArrayVariant(UA_Variant &variant, nlohmann::json *jsonValue, bool serializeStatusInformation) {
				switch (variant.type->typeKind) {
					SIMPLECASE(BOOLEAN, Boolean);
					SIMPLECASE(SBYTE, SByte);
					SIMPLECASE(BYTE, Byte);
					SIMPLECASE(INT16, Int16);
					SIMPLECASE(UINT16, UInt16);
					SIMPLECASE(INT32, Int32);
					SIMPLECASE(UINT32, UInt32);
					SIMPLECASE(INT64, Int64);
					SIMPLECASE(UINT64, UInt64);
					SIMPLECASE(FLOAT, Float);
					SIMPLECASE(DOUBLE, Double);
					SIMPLECASE(STRING, String);
					SIMPLECASE(DATETIME, DateTime);
					SIMPLECASE(LOCALIZEDTEXT, LocalizedText);
					SIMPLECASE(NODEID, NodeId);
					SIMPLECASE(EXTENSIONOBJECT, ExtensionObject);
					CASENOTIMPLEMENTED(GUID, Guid);
					CASENOTIMPLEMENTED(BYTESTRING, ByteString);
					CASENOTIMPLEMENTED(XMLELEMENT, XmlElement);
					CASENOTIMPLEMENTED(EXPANDEDNODEID, ExpandedNodeId);
					CASENOTIMPLEMENTED(STATUSCODE, StatusCode);
					CASENOTIMPLEMENTED(QUALIFIEDNAME, QualifiedName);
					CASENOTIMPLEMENTED(DATAVALUE, DataValue);
					CASENOTIMPLEMENTED(VARIANT, Variant);
					CASENOTIMPLEMENTED(DIAGNOSTICINFO, DiagnosticInfo);

					case UA_DATATYPEKIND_STRUCTURE: {
						if (strcmp(variant.type->typeName, "EUInformation") == 0) {
							VALUEFROMDATAARRAY(EUINFORMATION, EUInformation);
							break;
						} else if (strcmp(variant.type->typeName, "Range") == 0) {
							VALUEFROMDATAARRAY(RANGE, Range);
							break;
						} else {
							LOG(ERROR) << "Unknown data type. ";
							break;
						}
					}

					default: {
						LOG(ERROR) << "Unknown data type. ";
						break;
					}
				}

			}

			void UaDataValueToJsonValue::setValueFromDataValue(const UA_DataValue &dataValue,
															   bool serializeStatusInformation) {
				auto jsonValue = &m_value;
				if (serializeStatusInformation) {
					jsonValue = &m_value["value"];
				}
				
				UA_Variant variant; 
				UA_Variant_init(&variant);
				variant = dataValue.value;
				
				if (UA_Variant_isEmpty(&variant)) {
					return;
				}
				if (!UA_Variant_isScalar(&variant)) {
					UA_UInt32 x = variant.arrayLength;
					if (variant.arrayDimensionsSize == 0) {
						variant.arrayDimensionsSize = 1;
						variant.arrayDimensions = &x;
					}
					setValueFromArrayVariant(variant, jsonValue, serializeStatusInformation);
				} else {
					setValueFromScalarVariant(variant, jsonValue, serializeStatusInformation);
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
