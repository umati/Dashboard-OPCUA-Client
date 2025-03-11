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
#include "../Exceptions/OpcUaNonGoodStatusCodeException.hpp"
#include <iomanip>
#include "CustomDataTypes/types_machinery_result_generated_handling.h"
#include "CustomDataTypes/types_tightening_generated_handling.h"
#include "CustomDataTypes/types_ijt_base_generated.h"
#include "../deps/open62541/src/ua_types_encoding_binary.h"

namespace Umati {
namespace OpcUa {
namespace Converter {

UaDataValueToJsonValue::UaDataValueToJsonValue(const UA_DataValue &dataValue, bool serializeStatusInformation) {
  setValueFromDataValue(dataValue, serializeStatusInformation);
  if (serializeStatusInformation) {
    setStatusCodeFromDataValue(dataValue);
  }
}

void decodeJoiningResultMetaDataType(nlohmann::json *jsonValue, UA_JoiningResultMetaDataType *joiningResult, bool serializeStatusInformation) {
  UA_DataValue dataVal;
  UA_DataValue_init(&dataVal);
  if (joiningResult->assemblyType != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->assemblyType, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["ResultId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->associatedEntities != nullptr) {
    UA_Variant_setArray(
      &dataVal.value, joiningResult->associatedEntities, joiningResult->associatedEntitiesSize, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_ENTITYDATATYPE]);
    (*jsonValue)["AssociatedEntities"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->classification != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->classification, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["Classification"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->creationTime != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->creationTime, &UA_TYPES[UA_TYPES_DATETIME]);
    (*jsonValue)["CreationTime"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->description != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    (*jsonValue)["Description"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->extendedMetaData != nullptr) {
    UA_Variant_setArray(
      &dataVal.value, joiningResult->extendedMetaData, joiningResult->extendedMetaDataSize, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_KEYVALUEDATATYPE]);
    (*jsonValue)["ExtendedMetaData"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->externalConfigurationId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->externalConfigurationId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ExternalConfigurationId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->externalRecipeId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->externalRecipeId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ExternalRecipeId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->fileFormat != nullptr) {
    UA_Variant_setArray(&dataVal.value, joiningResult->fileFormat, joiningResult->fileFormatSize, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["FileFormat"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
}

void decodeJoiningResultDataType(nlohmann::json *jsonValue, UA_JoiningResultDataType *joiningResult, bool serializeStatusInformation) {
  UA_DataValue dataVal;
  UA_DataValue_init(&dataVal);
  if (joiningResult->errors != nullptr) {
    UA_Variant_setArray(
      &dataVal.value, joiningResult->errors, joiningResult->errorsSize, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_ERRORINFORMATIONDATATYPE]);
    (*jsonValue)["Errors"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->failingStepResultId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->failingStepResultId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["FailingStepResultId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->failureReason != nullptr) {
    UA_Variant_setScalar(&dataVal.value, joiningResult->failureReason, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["FailureReason"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->overallResultValues != nullptr) {
    UA_Variant_setArray(
      &dataVal.value,
      joiningResult->overallResultValues,
      joiningResult->overallResultValuesSize,
      &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_RESULTVALUEDATATYPE]);
    (*jsonValue)["OverallResultValues"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (joiningResult->stepResults != nullptr) {
    UA_Variant_setArray(
      &dataVal.value, joiningResult->stepResults, joiningResult->stepResultsSize, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_STEPRESULTDATATYPE]);
    (*jsonValue)["StepResults"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
}

void decodeEntityDataType(nlohmann::json *jsonValue, UA_EntityDataType &entity) {
  (*jsonValue)["EntityId"] = std::string((char *)entity.entityId.data, entity.entityId.length);
  (*jsonValue)["EntityType"] = entity.entityType;

  if (entity.description != nullptr) {
    (*jsonValue)["Description"] = std::string((char *)entity.description->data, entity.description->length);
  }
  if (entity.entityOriginId != nullptr) {
    (*jsonValue)["Description"] = std::string((char *)entity.entityOriginId->data, entity.entityOriginId->length);
  }
  if (entity.name != nullptr) {
    (*jsonValue)["Name"] = std::string((char *)entity.name->data, entity.name->length);
  }
  if (entity.isExternal != nullptr) {
    (*jsonValue)["Description"] = *(bool *)entity.isExternal;
  }
}

void decodeResultDataType(nlohmann::json *jsonValue, UA_ResultDataType *rd, bool serializeStatusInformation) {
  UA_DataValue dataVal;
  UA_DataValue_init(&dataVal);
  {
    UA_Variant_setScalar(&dataVal.value, &rd->resultMetaData, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
    (*jsonValue)["ResultMetaData"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->resultContent != nullptr) {
    UA_Variant_setArray(&dataVal.value, rd->resultContent, rd->resultContentSize, &UA_TYPES[UA_TYPES_VARIANT]);
    (*jsonValue)["ResultContent"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
}

void decodeResultEvaluationEnum(nlohmann::json *jsonValue, UA_ResultEvaluationEnum *rd, bool serializeStatusInformation) {
  switch (*rd) {
    case UA_RESULTEVALUATIONENUM_UNDEFINED: {
      (*jsonValue) = "UA_RESULTEVALUATIONENUM_UNDEFINED";
      break;
    }
    case UA_RESULTEVALUATIONENUM_OK: {
      (*jsonValue) = "UA_RESULTEVALUATIONENUM_OK";
      break;
    }
    case UA_RESULTEVALUATIONENUM_NOTOK: {
      (*jsonValue) = "UA_RESULTEVALUATIONENUM_NOTOK";
      break;
    }
    case UA_RESULTEVALUATIONENUM_NOTDECIDABLE: {
      (*jsonValue) = "UA_RESULTEVALUATIONENUM_NOTDECIDABLE";
      break;
    }
  }
}

void decodeStepResultDataType(nlohmann::json *jsonValue, UA_StepResultDataType *rd, bool serializeStatusInformation) {
  UA_DataValue dataVal;
  UA_DataValue_init(&dataVal);
  if (rd->name != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->name, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["Name"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->programStep != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->programStep, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ProgramStep"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->programStepId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->programStepId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ProgramStepId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->resultEvaluation != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->resultEvaluation, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
    (*jsonValue)["ResultEvaluation"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->startTimeOffset != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->startTimeOffset, &UA_TYPES[UA_TYPES_DOUBLE]);
    (*jsonValue)["StartTimeOffset"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  {
    UA_Variant_setScalar(&dataVal.value, &rd->stepResultId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["StepResultId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->stepResultValues != nullptr) {
    UA_Variant_setArray(&dataVal.value, rd->stepResultValues, rd->stepResultValuesSize, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_RESULTVALUEDATATYPE]);
    (*jsonValue)["StepResultValues"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->stepTraceId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->stepTraceId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["StepTraceId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
}

void decodeResultValueDataType(nlohmann::json *jsonValue, UA_ResultValueDataType *rd, bool serializeStatusInformation) {
  UA_DataValue dataVal;
  UA_DataValue_init(&dataVal);
  if (rd->engineeringUnits != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->engineeringUnits, &UA_TYPES[UA_TYPES_EUINFORMATION]);
    (*jsonValue)["EngineeringUnits"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->highLimit != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->highLimit, &UA_TYPES[UA_TYPES_DOUBLE]);
    (*jsonValue)["HighLimit"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->lowLimit != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->lowLimit, &UA_TYPES[UA_TYPES_DOUBLE]);
    (*jsonValue)["LowLimit"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  { (*jsonValue)["MeasuredValue"] = (double)rd->measuredValue; }
  if (rd->name != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->name, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["Name"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->parameterIdList != nullptr) {
    UA_Variant_setArray(&dataVal.value, rd->parameterIdList, rd->parameterIdListSize, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ParameterIdList"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->physicalQuantity != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->physicalQuantity, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["PhysicalQuantity"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->resultEvaluation != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->resultEvaluation, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
    (*jsonValue)["ResultEvaluation"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->resultStep != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->resultStep, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ResultStep"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->sensorId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->sensorId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["SensorId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->targetValue != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->targetValue, &UA_TYPES[UA_TYPES_DOUBLE]);
    (*jsonValue)["TargetValue"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->tracePointIndex != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->tracePointIndex, &UA_TYPES[UA_TYPES_INT32]);
    (*jsonValue)["TracePointIndex"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->tracePointTimeOffset != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->tracePointTimeOffset, &UA_TYPES[UA_TYPES_DOUBLE]);
    (*jsonValue)["TracePointTimeOffset"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->valueId != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->valueId, &UA_TYPES[UA_TYPES_STRING]);
    (*jsonValue)["ValueId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->valueTag != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->valueTag, &UA_TYPES[UA_TYPES_INT16]);
    (*jsonValue)["ValueTag"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->violationConsequence != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->violationConsequence, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["ViolationConsequence"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
  if (rd->violationType != nullptr) {
    UA_Variant_setScalar(&dataVal.value, rd->violationType, &UA_TYPES[UA_TYPES_BYTE]);
    (*jsonValue)["ViolationType"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
  }
}

void UaDataValueToJsonValue::setValueFromScalarVariant(UA_Variant &variant, nlohmann::json *jsonValue, bool serializeStatusInformation) {
  switch (variant.type->typeKind) {
    case UA_DATATYPEKIND_BOOLEAN: {
      UA_Boolean v(*(UA_Boolean *)variant.data);
      *jsonValue = static_cast<bool>(v);
      break;
    }

    case UA_DATATYPEKIND_SBYTE: {
      UA_SByte v(*(UA_SByte *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_BYTE: {
      UA_Byte v(*(UA_Byte *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_INT16: {
      UA_Int16 v(*(UA_Int16 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_UINT16: {
      UA_UInt16 v(*(UA_UInt16 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_INT32: {
      UA_Int32 v(*(UA_Int32 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_UINT32: {
      UA_UInt32 v(*(UA_UInt32 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_INT64: {
      UA_Int64 v(*(UA_Int64 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_UINT64: {
      UA_UInt64 v(*(UA_UInt64 *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_FLOAT: {
      UA_Float v(*(UA_Float *)variant.data);
      *jsonValue = v;
      break;
    }

    case UA_DATATYPEKIND_DOUBLE: {
      UA_Double v(*(UA_Double *)variant.data);
      *jsonValue = v;
      break;
    }
    case UA_DATATYPEKIND_STRING: {
      UA_String s(*(UA_String *)variant.data);
      *jsonValue = std::string((char *)s.data, s.length);
      break;
    }

    case UA_DATATYPEKIND_DATETIME: {
      UA_DateTime dateTime(*(UA_DateTime *)variant.data);
      auto dtStruct = UA_DateTime_toStruct(dateTime);
      // No to String() function. Using stringstream to build the string in the right format.
      std::stringstream dateTimeString;
      dateTimeString << dtStruct.year << "-" << dtStruct.month << "-" << dtStruct.day << "T" << dtStruct.hour << ":" << dtStruct.min << ":" << dtStruct.sec
                     << ":" << dtStruct.milliSec << "Z";

      *jsonValue = dateTimeString.str();
      break;
    }

    case UA_DATATYPEKIND_GUID: {
      UA_Guid guid(*(UA_Guid *)variant.data);
      std::stringstream str;
      str << std::hex << (uint32_t)guid.data1 << '-' << (uint16_t)guid.data2 << '-' << (uint16_t)guid.data3 << '-' << *(uint32_t *)&guid.data4[0] << '-'
          << *(uint32_t *)&guid.data4[4];
      *jsonValue = str.str();
      break;
    }

    case UA_DATATYPEKIND_BYTESTRING: {
      UA_ByteString bstring(*(UA_ByteString *)variant.data);
      std::stringstream str;
      for (int i = 0; i < bstring.length; i++) {
        str << bstring.data[i];
      }
      *jsonValue = str.str();

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
      nodeId = *(UA_NodeId *)variant.data;
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

    case UA_DATATYPEKIND_EXTENSIONOBJECT: {
      UA_ExtensionObject exObj(*(UA_ExtensionObject *)variant.data);
      *jsonValue = {};
      if (exObj.content.encoded.typeId.namespaceIndex != 0) {
        if (exObj.content.encoded.typeId.identifier.numeric == 3008) {
          LOG(INFO) << 'he';
        } else if (exObj.content.encoded.typeId.identifier.numeric == 5008) {
          LOG(INFO) << 'he';
        } else if (exObj.content.encoded.typeId.identifier.numeric == 5005) {
          UA_ResultMetaDataType resultMetaData;
          UA_StatusCode retval =
            UA_decodeBinary(&exObj.content.encoded.body, &resultMetaData, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE], NULL);
          UA_DataValue dataVal;
          UA_DataValue_init(&dataVal);
          {
            UA_Variant_setScalar(&dataVal.value, &resultMetaData.resultId, &UA_TYPES[UA_TYPES_STRING]);
            (*jsonValue)["ResultId"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }
          if (resultMetaData.resultState) {
            UA_Variant_setScalar(&dataVal.value, resultMetaData.resultState, &UA_TYPES[UA_TYPES_INT32]);
            (*jsonValue)["ResultState"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }
          if (resultMetaData.resultUri) {
            UA_Variant_setArray(&dataVal.value, resultMetaData.resultUri, resultMetaData.resultUriSize, &UA_TYPES[UA_TYPES_STRING]);
            (*jsonValue)["ResultUri"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }
          if (resultMetaData.fileFormat) {
            UA_Variant_setArray(&dataVal.value, resultMetaData.fileFormat, resultMetaData.fileFormatSize, &UA_TYPES[UA_TYPES_STRING]);
            (*jsonValue)["FileFormat"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }

        } else if (exObj.content.encoded.typeId.identifier.numeric == 5079 /* Encoding of EntityDataType */) {
          size_t offset = 0;
          UA_EntityDataType entity;
          UA_StatusCode retval =
            UA_decodeBinaryInternal(&exObj.content.encoded.body, &offset, &entity, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_ENTITYDATATYPE], NULL);
          decodeEntityDataType(jsonValue, entity);
        } else if (exObj.content.encoded.typeId.identifier.numeric == 5046 /* Encoding of JoiningResultMetaDataType */) {
          size_t offset = 0;
          UA_JoiningResultMetaDataType joiningResult;
          UA_StatusCode retval = UA_decodeBinaryInternal(
            &exObj.content.encoded.body, &offset, &joiningResult, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_JOININGRESULTMETADATATYPE], NULL);
          decodeJoiningResultMetaDataType(jsonValue, &joiningResult, serializeStatusInformation);
        } else if (exObj.content.encoded.typeId.identifier.numeric == 5049 /* Encoding of JoiningResultDataType */) {
          size_t offset = 0;
          UA_JoiningResultDataType joiningResult;
          UA_StatusCode retval = UA_decodeBinaryInternal(
            &exObj.content.encoded.body, &offset, &joiningResult, &UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_JOININGRESULTDATATYPE], NULL);
          decodeJoiningResultDataType(jsonValue, &joiningResult, serializeStatusInformation);
        } else {
          LOG(ERROR) << "Not implemented conversion from OpcUaType_ExtensionObject with custom structured data type: " << "ns="
                     << exObj.content.encoded.typeId.namespaceIndex << "i=" << exObj.content.encoded.typeId.identifier.numeric;
        }

        break;
      }

      switch (exObj.content.encoded.typeId.identifierType) {
        case UA_TYPES_RANGE: {
          UA_Range range(*(UA_Range *)exObj.content.decoded.data);
          (*jsonValue)["Low"] = range.low;
          (*jsonValue)["High"] = range.high;
          break;
        }

        case UA_TYPES_EUINFORMATION: {
          UA_EUInformation euInfo(*(UA_EUInformation *)variant.data);
          (*jsonValue)["NamespaceUri"] = std::string((char *)euInfo.namespaceUri.data, euInfo.namespaceUri.length);
          (*jsonValue)["UnitId"] = euInfo.unitId;
          UA_DataValue dataVal;
          UA_DataValue_init(&dataVal);
          {
            UA_Variant_setScalar(&dataVal.value, &euInfo.displayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            (*jsonValue)["DisplayName"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }
          {
            UA_Variant_setScalar(&dataVal.value, &euInfo.description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
            (*jsonValue)["Description"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
          }
          break;
        }

        case UA_TYPES_GUID: {
          UA_Guid guid(*(UA_Guid *)exObj.content.decoded.data);
          std::stringstream str;
          str << std::hex << guid.data1 << '-' << std::hex << guid.data2 << '-' << std::hex << guid.data3 << '-' << std::hex << guid.data4;
          (*jsonValue) = str.str();
          break;
        }

        case UA_TYPES_TIMEZONEDATATYPE: {
          UA_TimeZoneDataType tz(*(UA_TimeZoneDataType *)exObj.content.decoded.data);
          (*jsonValue)["daylightSavingInOffset"] = tz.daylightSavingInOffset;
          (*jsonValue)["offset"] = tz.offset;
          break;
        }

        default: {
          if (exObj.encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY) {
            LOG(ERROR) << "Internal decoding error in open62541, might be a unknown custom datatype";
          } else {
            LOG(ERROR) << "Not implemented conversion from type: " << exObj.content.encoded.body.data;
          }
        }
      }
      break;
    }

    case UA_DATATYPEKIND_DATAVALUE: {
      LOG(ERROR) << "Not implemented conversion to OpcUaType_DataValue. ";
      break;
    }

    case UA_DATATYPEKIND_VARIANT: {
      UA_Variant var = *(UA_Variant *)variant.data;
      UA_DataValue dataVal;
      UA_DataValue_init(&dataVal);
      dataVal.value = var;
      { (*jsonValue) = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue(); }
      break;
    }

    case UA_DATATYPEKIND_DIAGNOSTICINFO: {
      LOG(ERROR) << "Not implemented conversion to OpcUaType_DiagnosticInfo. ";
      break;
    }

    case UA_DATATYPEKIND_QUALIFIEDNAME: {
      UA_QualifiedName qualifiedName(*(UA_QualifiedName *)variant.data);
      (*jsonValue)["namespaceIndex"] = qualifiedName.namespaceIndex;
      (*jsonValue)["name"] = std::string((char *)qualifiedName.name.data, qualifiedName.name.length);
      break;
    }

    case UA_DATATYPEKIND_LOCALIZEDTEXT: {
      UA_LocalizedText localText(*(UA_LocalizedText *)variant.data);
      *jsonValue = {};
      // FIX_BEGIN (FIX_3)
      // Inserted nullchecks
      // Added a max length because length determination on some Servers return invalid high length numbers
      std::size_t maxLength = 4000;
      if (localText.locale.data != nullptr && localText.locale.length < maxLength) {
        (*jsonValue)["locale"] = std::string((char *)localText.locale.data, localText.locale.length);
      }
      if (localText.text.data != nullptr && localText.text.length < maxLength) {
        (*jsonValue)["text"] = std::string((char *)localText.text.data, localText.text.length);
      }
      // FIX_END
      break;
    }

    case UA_DATATYPEKIND_STRUCTURE: {
      if (strcmp(variant.type->typeName, "EUInformation") == 0) {
        UA_EUInformation euInfo(*(UA_EUInformation *)variant.data);
        (*jsonValue)["NamespaceUri"] = std::string((char *)euInfo.namespaceUri.data, euInfo.namespaceUri.length);
        (*jsonValue)["UnitId"] = euInfo.unitId;
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        {
          UA_Variant_setScalar(&dataVal.value, &euInfo.displayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
          (*jsonValue)["DisplayName"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
        }
        {
          UA_Variant_setScalar(&dataVal.value, &euInfo.description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
          (*jsonValue)["Description"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
        }
        break;
      } else if (strcmp(variant.type->typeName, "Range") == 0) {
        UA_Range range(*(UA_Range *)variant.data);
        (*jsonValue)["Low"] = range.low;
        (*jsonValue)["High"] = range.high;
        break;
      } else if (strcmp(variant.type->typeName, "TimeZoneDataType") == 0) {
        UA_TimeZoneDataType tz(*(UA_TimeZoneDataType *)variant.data);
        (*jsonValue)["daylightSavingInOffset"] = tz.daylightSavingInOffset;
        (*jsonValue)["offset"] = tz.offset;
        break;
      } else if (strcmp(variant.type->typeName, "KeyValueDataType") == 0) {
        UA_KeyValueDataType kv(*(UA_KeyValueDataType *)variant.data);
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        (*jsonValue)["Key"] = std::string((char *)kv.key.data, kv.key.length);
        {
          UA_Variant_setScalar(&dataVal.value, &kv.value, &UA_TYPES[UA_TYPES_VARIANT]);
          (*jsonValue)["Value"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
        }
        break;
      } else if (strcmp(variant.type->typeName, "ProcessingTimesDataType") == 0) {
        UA_ProcessingTimesDataType tz(*(UA_ProcessingTimesDataType *)variant.data);
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        if (tz.acquisitionDuration) {
          (*jsonValue)["acquisitionDuration"] = *(double *)tz.acquisitionDuration;
        }
        {
          UA_Variant_setScalar(&dataVal.value, &tz.endTime, &UA_TYPES[UA_TYPES_DATETIME]);
          (*jsonValue)["EndTime"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
        }
        if (tz.processingDuration) {
          (*jsonValue)["ProcessingDuration"] = *(double *)tz.processingDuration;
        }
        {
          UA_Variant_setScalar(&dataVal.value, &tz.startTime, &UA_TYPES[UA_TYPES_DATETIME]);
          (*jsonValue)["StartTime"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
        }
        break;
      } else if (strcmp(variant.type->typeName, "ResultDataType") == 0) {
        UA_ResultDataType rd(*(UA_ResultDataType *)variant.data);
        decodeResultDataType(jsonValue, &rd, serializeStatusInformation);

      } else {
        LOG(ERROR) << "Unknown data type. ";
        break;
      }
    }

    default: {
      if (strcmp(variant.type->typeName, UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE].typeName) == 0) {
        UA_IJT_ProcessingTimesDataType ptime(*(UA_IJT_ProcessingTimesDataType *)variant.data);
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        if (ptime.acquisitionDuration) (*jsonValue)["AcquisitionDuration"] = *ptime.acquisitionDuration;
        if (ptime.processingDuration) (*jsonValue)["ProcessingDuration"] = *ptime.processingDuration;

        UA_Variant_setScalar(&dataVal.value, &ptime.startTime, &UA_TYPES[UA_TYPES_DATETIME]);
        (*jsonValue)["StartTime"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();

        UA_Variant_setScalar(&dataVal.value, &ptime.endTime, &UA_TYPES[UA_TYPES_DATETIME]);
        (*jsonValue)["EndTime"] = UaDataValueToJsonValue(dataVal, serializeStatusInformation).getValue();
      } else if (strcmp(variant.type->typeName, UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_ENTITYDATATYPE].typeName) == 0) {
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        UA_EntityDataType entity = *(UA_EntityDataType *)variant.data;
        decodeEntityDataType(jsonValue, entity);
      } else if (strcmp(variant.type->typeName, UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE].typeName) == 0) {
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        UA_ResultDataType entity = *(UA_ResultDataType *)variant.data;
        decodeResultDataType(jsonValue, &entity, serializeStatusInformation);
      } else if (strcmp(variant.type->typeName, UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_RESULTVALUEDATATYPE].typeName) == 0) {
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        UA_ResultValueDataType entity = *(UA_ResultValueDataType *)variant.data;
        decodeResultValueDataType(jsonValue, &entity, serializeStatusInformation);
      } else if (strcmp(variant.type->typeName, UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM].typeName) == 0) {
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        UA_ResultEvaluationEnum entity = *(UA_ResultEvaluationEnum *)variant.data;
        decodeResultEvaluationEnum(jsonValue, &entity, serializeStatusInformation);
      } else if (strcmp(variant.type->typeName, UA_TYPES_IJT_GENERATED[UA_TYPES_IJT_GENERATED_STEPRESULTDATATYPE].typeName) == 0) {
        UA_DataValue dataVal;
        UA_DataValue_init(&dataVal);
        UA_StepResultDataType entity = *(UA_StepResultDataType *)variant.data;
        decodeStepResultDataType(jsonValue, &entity, serializeStatusInformation);
      } else {
        LOG(ERROR) << "Unknown data type. ";
      }
      break;
    }
  }
}

template <typename T>
void UaDataValueToJsonValue::getValueFromDataValueArray(
  const UA_Variant *variant, UA_UInt32 dimensionNumber, nlohmann::json *j, T *variantData, bool serializeStatusInformation) {
  if (dimensionNumber == variant->arrayDimensionsSize - 1) {
    for (int i = 0; i < variant->arrayDimensions[dimensionNumber]; i++) {
      nlohmann::json jsonValue;
      UA_Variant var = {
        variant->type, /* The data type description */
        variant->storageType,
        0,                            /* The number of elements in the data array */
        (void *)(&variantData[i]),    /* Points to the scalar or array data */
        variant->arrayDimensionsSize, /* The number of dimensions */
        NULL                          /* Pointer to dimensionsArray */
      };
      setValueFromScalarVariant(var, &jsonValue, serializeStatusInformation);
      j->push_back(jsonValue);
    }
    return;
  }
  UA_UInt32 offset = 1;
  for (UA_UInt32 i = dimensionNumber + 1; i < variant->arrayDimensionsSize - 1; i++) {
    offset = offset * variant->arrayDimensions[i];
  }
  for (UA_UInt32 i = 0; i < variant->arrayDimensions[dimensionNumber]; i++) {
    auto nestedj = nlohmann::json::array();
    getValueFromDataValueArray<T>(variant, dimensionNumber + 1, &nestedj, &variantData[i * offset], serializeStatusInformation);
    j->push_back(nestedj);
  }
}

#define VALUEFROMDATAARRAY(ENUM, VAR)     \
  UA_##VAR *v = (UA_##VAR *)variant.data; \
  getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), jsonValue, v, serializeStatusInformation);

#define SIMPLECASE(ENUM, VAR)                                                                               \
  case UA_DATATYPEKIND_##ENUM: {                                                                            \
    UA_##VAR *v = (UA_##VAR *)variant.data;                                                                 \
    getValueFromDataValueArray<UA_##VAR>(&variant, UA_UInt32(0), jsonValue, v, serializeStatusInformation); \
    break;                                                                                                  \
  }

#define CASENOTIMPLEMENTED(ENUM, VAR)                        \
  case UA_DATATYPEKIND_##ENUM: {                             \
    LOG(ERROR) << "Not implented conversion to UA_" << #VAR; \
    break;                                                   \
  }

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
    SIMPLECASE(QUALIFIEDNAME, QualifiedName);
    SIMPLECASE(VARIANT, Variant);
    CASENOTIMPLEMENTED(GUID, Guid);
    CASENOTIMPLEMENTED(BYTESTRING, ByteString);
    CASENOTIMPLEMENTED(XMLELEMENT, XmlElement);
    CASENOTIMPLEMENTED(EXPANDEDNODEID, ExpandedNodeId);
    CASENOTIMPLEMENTED(STATUSCODE, StatusCode);
    CASENOTIMPLEMENTED(DATAVALUE, DataValue);
    CASENOTIMPLEMENTED(DIAGNOSTICINFO, DiagnosticInfo);

    case UA_DATATYPEKIND_STRUCTURE: {
      if (strcmp(variant.type->typeName, "EUInformation") == 0) {
        VALUEFROMDATAARRAY(EUINFORMATION, EUInformation);
        break;
      } else if (strcmp(variant.type->typeName, "Range") == 0) {
        VALUEFROMDATAARRAY(RANGE, Range);
        break;
      } else if (strcmp(variant.type->typeName, "KeyValueDataType") == 0) {
        VALUEFROMDATAARRAY(KEYVALUEDATATYPE, KeyValueDataType);
        break;
      } else {
        LOG(ERROR) << "Unknown data type. ";
        break;
      }
    }

    case UA_DATATYPEKIND_OPTSTRUCT: {
      if (strcmp(variant.type->typeName, "EntityDataType") == 0) {
        VALUEFROMDATAARRAY(ENTITYDATATYPE, EntityDataType);
        break;
      } else if (strcmp(variant.type->typeName, "ResultDataType") == 0) {
        VALUEFROMDATAARRAY(RESULTDATATYPE, ResultDataType);
        break;
      } else if (strcmp(variant.type->typeName, "ResultValueDataType") == 0) {
        VALUEFROMDATAARRAY(RESULTVALUEDATATYPE, ResultValueDataType);
        break;
      } else if (strcmp(variant.type->typeName, "StepResultDataType") == 0) {
        VALUEFROMDATAARRAY(STEPRESULTDATATYPE, StepResultDataType);
        break;
      } else {
        LOG(ERROR) << "Unknown OptStruct. ";
        break;
      }
      break;
    }

    default: {
      LOG(ERROR) << "Unknown data type. ";
      break;
    }
  }
}

void UaDataValueToJsonValue::setValueFromDataValue(const UA_DataValue &dataValue, bool serializeStatusInformation) {
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
  jsonStatusCode["isGood"] = (dataValue.status == UA_STATUSCODE_GOOD) ? UA_TRUE : UA_FALSE;
}
}  // namespace Converter
}  // namespace OpcUa
}  // namespace Umati
