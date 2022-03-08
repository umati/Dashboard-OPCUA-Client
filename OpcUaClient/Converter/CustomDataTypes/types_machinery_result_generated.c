/**********************************
 * Autogenerated -- do not modify *
 **********************************/

#include "types_machinery_result_generated.h"

/* ProcessingTimesDataType */
static UA_DataTypeMember ProcessingTimesDataType_members[4] = {
{
    UA_TYPENAME("StartTime") /* .memberName */
    &UA_TYPES[UA_TYPES_DATETIME], /* .memberType */
    0, /* .padding */
    false, /* .isArray */
    false  /* .isOptional */
},
{
    UA_TYPENAME("EndTime") /* .memberName */
    &UA_TYPES[UA_TYPES_DATETIME], /* .memberType */
    offsetof(UA_ProcessingTimesDataType, endTime) - offsetof(UA_ProcessingTimesDataType, startTime) - sizeof(UA_DateTime), /* .padding */
    false, /* .isArray */
    false  /* .isOptional */
},
{
    UA_TYPENAME("AcquisitionDuration") /* .memberName */
    &UA_TYPES[UA_TYPES_DOUBLE], /* .memberType */
    offsetof(UA_ProcessingTimesDataType, acquisitionDuration) - offsetof(UA_ProcessingTimesDataType, endTime) - sizeof(UA_DateTime), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ProcessingDuration") /* .memberName */
    &UA_TYPES[UA_TYPES_DOUBLE], /* .memberType */
    offsetof(UA_ProcessingTimesDataType, processingDuration) - offsetof(UA_ProcessingTimesDataType, acquisitionDuration) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},};

/* ResultTransferOptionsDataType */
static UA_DataTypeMember ResultTransferOptionsDataType_members[1] = {
{
    UA_TYPENAME("ResultId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    0, /* .padding */
    false, /* .isArray */
    false  /* .isOptional */
},};

/* ResultEvaluationEnum */
#define ResultEvaluationEnum_members NULL

/* ResultMetaDataType */
static UA_DataTypeMember ResultMetaDataType_members[20] = {
{
    UA_TYPENAME("ResultId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    0, /* .padding */
    false, /* .isArray */
    false  /* .isOptional */
},
{
    UA_TYPENAME("HasTransferableDataOnFile") /* .memberName */
    &UA_TYPES[UA_TYPES_BOOLEAN], /* .memberType */
    offsetof(UA_ResultMetaDataType, hasTransferableDataOnFile) - offsetof(UA_ResultMetaDataType, resultId) - sizeof(UA_String), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("IsPartial") /* .memberName */
    &UA_TYPES[UA_TYPES_BOOLEAN], /* .memberType */
    offsetof(UA_ResultMetaDataType, isPartial) - offsetof(UA_ResultMetaDataType, hasTransferableDataOnFile) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("IsSimulated") /* .memberName */
    &UA_TYPES[UA_TYPES_BOOLEAN], /* .memberType */
    offsetof(UA_ResultMetaDataType, isSimulated) - offsetof(UA_ResultMetaDataType, isPartial) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ResultState") /* .memberName */
    &UA_TYPES[UA_TYPES_INT32], /* .memberType */
    offsetof(UA_ResultMetaDataType, resultState) - offsetof(UA_ResultMetaDataType, isSimulated) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("StepId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, stepId) - offsetof(UA_ResultMetaDataType, resultState) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("PartId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, partId) - offsetof(UA_ResultMetaDataType, stepId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ExternalRecipeId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, externalRecipeId) - offsetof(UA_ResultMetaDataType, partId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("InternalRecipeId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, internalRecipeId) - offsetof(UA_ResultMetaDataType, externalRecipeId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ProductId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, productId) - offsetof(UA_ResultMetaDataType, internalRecipeId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ExternalConfigurationId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, externalConfigurationId) - offsetof(UA_ResultMetaDataType, productId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("InternalConfigurationId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, internalConfigurationId) - offsetof(UA_ResultMetaDataType, externalConfigurationId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("JobId") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, jobId) - offsetof(UA_ResultMetaDataType, internalConfigurationId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("CreationTime") /* .memberName */
    &UA_TYPES[UA_TYPES_DATETIME], /* .memberType */
    offsetof(UA_ResultMetaDataType, creationTime) - offsetof(UA_ResultMetaDataType, jobId) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ProcessingTimes") /* .memberName */
    &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE], /* .memberType */
    offsetof(UA_ResultMetaDataType, processingTimes) - offsetof(UA_ResultMetaDataType, creationTime) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ResultUri") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, resultUriSize) - offsetof(UA_ResultMetaDataType, processingTimes) - sizeof(void *), /* .padding */
    true, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ResultEvaluation") /* .memberName */
    &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM], /* .memberType */
    offsetof(UA_ResultMetaDataType, resultEvaluation) - offsetof(UA_ResultMetaDataType, resultUri) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ResultEvaluationCode") /* .memberName */
    &UA_TYPES[UA_TYPES_INT32], /* .memberType */
    offsetof(UA_ResultMetaDataType, resultEvaluationCode) - offsetof(UA_ResultMetaDataType, resultEvaluation) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("ResultEvaluationDetails") /* .memberName */
    &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], /* .memberType */
    offsetof(UA_ResultMetaDataType, resultEvaluationDetails) - offsetof(UA_ResultMetaDataType, resultEvaluationCode) - sizeof(void *), /* .padding */
    false, /* .isArray */
    true  /* .isOptional */
},
{
    UA_TYPENAME("FileFormat") /* .memberName */
    &UA_TYPES[UA_TYPES_STRING], /* .memberType */
    offsetof(UA_ResultMetaDataType, fileFormatSize) - offsetof(UA_ResultMetaDataType, resultEvaluationDetails) - sizeof(void *), /* .padding */
    true, /* .isArray */
    true  /* .isOptional */
},};

/* ResultDataType */
static UA_DataTypeMember ResultDataType_members[2] = {
{
    UA_TYPENAME("ResultMetaData") /* .memberName */
    &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE], /* .memberType */
    0, /* .padding */
    false, /* .isArray */
    false  /* .isOptional */
},
{
    UA_TYPENAME("ResultContent") /* .memberName */
    &UA_TYPES[UA_TYPES_VARIANT], /* .memberType */
    offsetof(UA_ResultDataType, resultContentSize) - offsetof(UA_ResultDataType, resultMetaData) - sizeof(UA_ResultMetaDataType), /* .padding */
    true, /* .isArray */
    true  /* .isOptional */
},};
const UA_DataType UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_COUNT] = {
/* ProcessingTimesDataType */
{
    UA_TYPENAME("ProcessingTimesDataType") /* .typeName */
    {8, UA_NODEIDTYPE_NUMERIC, {3006LU}}, /* .typeId */
    {8, UA_NODEIDTYPE_NUMERIC, {5003LU}}, /* .binaryEncodingId */
    sizeof(UA_ProcessingTimesDataType), /* .memSize */
    UA_DATATYPEKIND_OPTSTRUCT, /* .typeKind */
    false, /* .pointerFree */
    false, /* .overlayable */
    4, /* .membersSize */
    ProcessingTimesDataType_members  /* .members */
},
/* ResultTransferOptionsDataType */
{
    UA_TYPENAME("ResultTransferOptionsDataType") /* .typeName */
    {8, UA_NODEIDTYPE_NUMERIC, {3005LU}}, /* .typeId */
    {8, UA_NODEIDTYPE_NUMERIC, {5001LU}}, /* .binaryEncodingId */
    sizeof(UA_ResultTransferOptionsDataType), /* .memSize */
    UA_DATATYPEKIND_STRUCTURE, /* .typeKind */
    false, /* .pointerFree */
    false, /* .overlayable */
    1, /* .membersSize */
    ResultTransferOptionsDataType_members  /* .members */
},
/* ResultEvaluationEnum */
{
    UA_TYPENAME("ResultEvaluationEnum") /* .typeName */
    {8, UA_NODEIDTYPE_NUMERIC, {3002LU}}, /* .typeId */
    {8, UA_NODEIDTYPE_NUMERIC, {0}}, /* .binaryEncodingId */
    sizeof(UA_ResultEvaluationEnum), /* .memSize */
    UA_DATATYPEKIND_ENUM, /* .typeKind */
    true, /* .pointerFree */
    UA_BINARY_OVERLAYABLE_INTEGER, /* .overlayable */
    0, /* .membersSize */
    ResultEvaluationEnum_members  /* .members */
},
/* ResultMetaDataType */
{
    UA_TYPENAME("ResultMetaDataType") /* .typeName */
    {8, UA_NODEIDTYPE_NUMERIC, {3007LU}}, /* .typeId */
    {8, UA_NODEIDTYPE_NUMERIC, {5005LU}}, /* .binaryEncodingId */
    sizeof(UA_ResultMetaDataType), /* .memSize */
    UA_DATATYPEKIND_OPTSTRUCT, /* .typeKind */
    false, /* .pointerFree */
    false, /* .overlayable */
    20, /* .membersSize */
    ResultMetaDataType_members  /* .members */
},
/* ResultDataType */
{
    UA_TYPENAME("ResultDataType") /* .typeName */
    {8, UA_NODEIDTYPE_NUMERIC, {3008LU}}, /* .typeId */
    {8, UA_NODEIDTYPE_NUMERIC, {5008LU}}, /* .binaryEncodingId */
    sizeof(UA_ResultDataType), /* .memSize */
    UA_DATATYPEKIND_OPTSTRUCT, /* .typeKind */
    false, /* .pointerFree */
    false, /* .overlayable */
    2, /* .membersSize */
    ResultDataType_members  /* .members */
},
};
