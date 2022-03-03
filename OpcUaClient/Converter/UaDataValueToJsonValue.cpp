 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

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

			void UaDataValueToJsonValue::setValueFromScalarVariant(UA_Variant &variant, nlohmann::json &jsonValue, bool serializeStatusInformation) {
				switch (variant.type->typeKind) {

					case UA_DATATYPEKIND_BOOLEAN: {
						UA_Boolean v(*(UA_Boolean*)variant.data);
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
						UA_Int64 v(*(UA_Int64*)variant.data);
						jsonValue = v;
						break;
					}

					case UA_DATATYPEKIND_UINT64: {
						UA_UInt64 v(*(UA_UInt64*)variant.data);
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
						UA_DateTime dateTime(*(UA_DateTime*)variant.data);
						auto dtStruct = UA_DateTime_toStruct(dateTime);
						//No to String() function. Using stringstream to build the string in the right format.
						std::stringstream dateTimeString;
						dateTimeString << dtStruct.year << "-" << dtStruct.month << "-" << dtStruct.day << "T"
						<< dtStruct.hour << ":" << dtStruct.min << ":" << dtStruct.sec << ":" << dtStruct.milliSec << "Z";

						jsonValue = dateTimeString.str();
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
						jsonValue = {};
						jsonValue["locale"] = std::string((char*)localText.locale.data,localText.locale.length);
						jsonValue["text"] =  std::string((char*)localText.text.data,localText.text.length);
						break;
					}
					UA_NodeId x;
					
					case UA_DATATYPEKIND_EXTENSIONOBJECT: {
						UA_ExtensionObject exObj(*(UA_ExtensionObject*)variant.data);
						jsonValue = {};
						if (exObj.content.encoded.typeId.namespaceIndex != 0) {
							UA_UInt32* x = (UA_UInt32*) &exObj.content.encoded.body;
		
							/*
							exObj.content.encoded.body.data.
							struct ProcessingTimesDataType {
								UA_DateTime StartTime;		   // M
								UA_DateTime EndTime;		   // M
								UA_Double AcquisitionDuration; // O
								UA_Double ProccessingDuration; // O
							};
							struct ResultMetaDataType {
								UA_String ResultId;						// 0
								UA_Boolean HasTransferableDataOnFile;	// 0
								UA_Boolean IsPartial;					// 0
								UA_Boolean IsSimulated;					// 0
								UA_Int32 ResultState;					// 0
								UA_String StepId;						// 0
								UA_String PartId;						// 0
								UA_String ExternalRecipeId;				// 0
								UA_String InternalRecipeId;				// 0
								UA_String ProductId;					// 0
								UA_String ExternalConfigurationId;		// 0
								UA_String InternalConfigurationId;		// 0
								UA_String JobId;						// 0
								UA_DateTime CreationTime;				// 0
								ProcessingTimesDataType ProcessingTimes;// 0
								UA_ListOfString ResultUri;				// 0
								ResultEvaluationEnum ResultEvaluation;	// 0
								UA_Int32 ResultEvaluationCode;			// 0	
								UA_LocalizedText ResultEvaluationDetails;// 0
								UA_ListOfString FileFormat;				// 0
							};

							struct ResultDataType {
								ResultMetaDataType ResultMetaData; //Liste
								UA_Variant ResultContent; //Liste

							}
							*/
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
								UA_EUInformation euInfo(*(UA_EUInformation*)variant.data);
								jsonValue["namespaceUri"] = std::string((char*)euInfo.namespaceUri.data,euInfo.namespaceUri.length);
								jsonValue["unitId"] = euInfo.unitId;
								UA_DataValue dataVal;
								UA_DataValue_init(&dataVal);
								{
									UA_Variant_setScalar(&dataVal.value, &euInfo.displayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
									jsonValue["displayName"] = UaDataValueToJsonValue(
										dataVal,
										serializeStatusInformation)
										.getValue();
								}
								{
									UA_Variant_setScalar(&dataVal.value, &euInfo.description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
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

					case UA_DATATYPEKIND_STRUCTURE: {

						if(strcmp(variant.type->typeName, "EUInformation")== 0){
								UA_EUInformation euInfo(*(UA_EUInformation*)variant.data);
								jsonValue["namespaceUri"] = std::string((char*)euInfo.namespaceUri.data,euInfo.namespaceUri.length);
								jsonValue["unitId"] = euInfo.unitId;
								UA_DataValue dataVal;
								UA_DataValue_init(&dataVal);
								{
									UA_Variant_setScalar(&dataVal.value, &euInfo.displayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
									jsonValue["displayName"] = UaDataValueToJsonValue(
										dataVal,
										serializeStatusInformation)
										.getValue();
								}
								{
									UA_Variant_setScalar(&dataVal.value, &euInfo.description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
									jsonValue["description"] = UaDataValueToJsonValue(
										dataVal,
										serializeStatusInformation)
										.getValue();
								}
								break;
							}else if (strcmp(variant.type->typeName, "Range")== 0){
								UA_Range range(*(UA_Range*)variant.data);
								jsonValue["low"] = range.low;
								jsonValue["high"] = range.high;
								break;
						}else{
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
		

			template<typename T>
			void UaDataValueToJsonValue::getValueFromDataValueArray(const UA_Variant *variant, UA_UInt32 dimensionNumber,
																	nlohmann::json *j, T *variantData, bool serializeStatusInformation) {
				if (dimensionNumber == variant->arrayDimensionsSize - 1) {
					for(int i = 0; i < variant->arrayDimensions[dimensionNumber]; i++) {												
						nlohmann::json jsonValue;
						UA_Variant var = {
							variant->type,  			/* The data type description */
							variant->storageType,
							0,           				/* The number of elements in the data array */
							(void*) (&variantData[i]), /* Points to the scalar or array data */
							variant->arrayDimensionsSize, /* The number of dimensions */
							NULL						/* Pointer to dimensionsArray */
						};
						setValueFromScalarVariant(var, jsonValue, serializeStatusInformation);
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
		getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), &jsonValue, v, serializeStatusInformation); \

#define SIMPLECASE(ENUM, VAR) case UA_DATATYPEKIND_##ENUM: { \
		UA_##VAR *v = (UA_##VAR*) variant.data; \
		getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), &jsonValue, v, serializeStatusInformation); \
		break; }

#define CASENOTIMPLEMENTED(ENUM, VAR) case UA_DATATYPEKIND_##ENUM: { \
		LOG(ERROR) << "Not implented conversion to UA_" << #VAR; \
		break; }

			void UaDataValueToJsonValue::setValueFromArrayVariant(UA_Variant &variant, nlohmann::json &jsonValue, bool serializeStatusInformation) {
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
				auto &jsonValue = m_value;
				if (serializeStatusInformation) {
					jsonValue = m_value["value"];
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
