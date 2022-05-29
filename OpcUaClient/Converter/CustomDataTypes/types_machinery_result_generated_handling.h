/**********************************
 * Autogenerated -- do not modify *
 **********************************/

#ifndef TYPES_MACHINERY_RESULT_GENERATED_HANDLING_H_
#define TYPES_MACHINERY_RESULT_GENERATED_HANDLING_H_

#include "types_machinery_result_generated.h"

_UA_BEGIN_DECLS

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# pragma GCC diagnostic ignored "-Wmissing-braces"
#endif


/* ProcessingTimesDataType */
static UA_INLINE void
UA_ProcessingTimesDataType_init(UA_ProcessingTimesDataType *p) {
    memset(p, 0, sizeof(UA_ProcessingTimesDataType));
}

static UA_INLINE UA_ProcessingTimesDataType *
UA_ProcessingTimesDataType_new(void) {
    return (UA_ProcessingTimesDataType*)UA_new(&UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ProcessingTimesDataType_copy(const UA_ProcessingTimesDataType *src, UA_ProcessingTimesDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE]);
}

UA_DEPRECATED static UA_INLINE void
UA_ProcessingTimesDataType_deleteMembers(UA_ProcessingTimesDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE void
UA_ProcessingTimesDataType_clear(UA_ProcessingTimesDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE void
UA_ProcessingTimesDataType_delete(UA_ProcessingTimesDataType *p) {
    UA_delete(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_PROCESSINGTIMESDATATYPE]);
}

/* ResultTransferOptionsDataType */
static UA_INLINE void
UA_ResultTransferOptionsDataType_init(UA_ResultTransferOptionsDataType *p) {
    memset(p, 0, sizeof(UA_ResultTransferOptionsDataType));
}

static UA_INLINE UA_ResultTransferOptionsDataType *
UA_ResultTransferOptionsDataType_new(void) {
    return (UA_ResultTransferOptionsDataType*)UA_new(&UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTTRANSFEROPTIONSDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ResultTransferOptionsDataType_copy(const UA_ResultTransferOptionsDataType *src, UA_ResultTransferOptionsDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTTRANSFEROPTIONSDATATYPE]);
}

UA_DEPRECATED static UA_INLINE void
UA_ResultTransferOptionsDataType_deleteMembers(UA_ResultTransferOptionsDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTTRANSFEROPTIONSDATATYPE]);
}

static UA_INLINE void
UA_ResultTransferOptionsDataType_clear(UA_ResultTransferOptionsDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTTRANSFEROPTIONSDATATYPE]);
}

static UA_INLINE void
UA_ResultTransferOptionsDataType_delete(UA_ResultTransferOptionsDataType *p) {
    UA_delete(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTTRANSFEROPTIONSDATATYPE]);
}

/* ResultEvaluationEnum */
static UA_INLINE void
UA_ResultEvaluationEnum_init(UA_ResultEvaluationEnum *p) {
    memset(p, 0, sizeof(UA_ResultEvaluationEnum));
}

static UA_INLINE UA_ResultEvaluationEnum *
UA_ResultEvaluationEnum_new(void) {
    return (UA_ResultEvaluationEnum*)UA_new(&UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
}

static UA_INLINE UA_StatusCode
UA_ResultEvaluationEnum_copy(const UA_ResultEvaluationEnum *src, UA_ResultEvaluationEnum *dst) {
    return UA_copy(src, dst, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
}

UA_DEPRECATED static UA_INLINE void
UA_ResultEvaluationEnum_deleteMembers(UA_ResultEvaluationEnum *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
}

static UA_INLINE void
UA_ResultEvaluationEnum_clear(UA_ResultEvaluationEnum *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
}

static UA_INLINE void
UA_ResultEvaluationEnum_delete(UA_ResultEvaluationEnum *p) {
    UA_delete(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTEVALUATIONENUM]);
}

/* ResultMetaDataType */
static UA_INLINE void
UA_ResultMetaDataType_init(UA_ResultMetaDataType *p) {
    memset(p, 0, sizeof(UA_ResultMetaDataType));
}

static UA_INLINE UA_ResultMetaDataType *
UA_ResultMetaDataType_new(void) {
    return (UA_ResultMetaDataType*)UA_new(&UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ResultMetaDataType_copy(const UA_ResultMetaDataType *src, UA_ResultMetaDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE]);
}

UA_DEPRECATED static UA_INLINE void
UA_ResultMetaDataType_deleteMembers(UA_ResultMetaDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE]);
}

static UA_INLINE void
UA_ResultMetaDataType_clear(UA_ResultMetaDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE]);
}

static UA_INLINE void
UA_ResultMetaDataType_delete(UA_ResultMetaDataType *p) {
    UA_delete(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTMETADATATYPE]);
}

/* ResultDataType */
static UA_INLINE void
UA_ResultDataType_init(UA_ResultDataType *p) {
    memset(p, 0, sizeof(UA_ResultDataType));
}

static UA_INLINE UA_ResultDataType *
UA_ResultDataType_new(void) {
    return (UA_ResultDataType*)UA_new(&UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_ResultDataType_copy(const UA_ResultDataType *src, UA_ResultDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE]);
}

UA_DEPRECATED static UA_INLINE void
UA_ResultDataType_deleteMembers(UA_ResultDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE]);
}

static UA_INLINE void
UA_ResultDataType_clear(UA_ResultDataType *p) {
    UA_clear(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE]);
}

static UA_INLINE void
UA_ResultDataType_delete(UA_ResultDataType *p) {
    UA_delete(p, &UA_TYPES_MACHINERY_RESULT[UA_TYPES_MACHINERY_RESULT_RESULTDATATYPE]);
}

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic pop
#endif

_UA_END_DECLS

#endif /* TYPES_MACHINERY_RESULT_GENERATED_HANDLING_H_ */