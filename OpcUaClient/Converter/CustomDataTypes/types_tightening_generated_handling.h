/**********************************
 * Autogenerated -- do not modify *
 **********************************/

#ifndef TYPES_TIGHTENING_GENERATED_HANDLING_H_
#define TYPES_TIGHTENING_GENERATED_HANDLING_H_

#include "types_tightening_generated.h"

_UA_BEGIN_DECLS

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# pragma GCC diagnostic ignored "-Wmissing-braces"
#endif


/* ProcessingTimesDataType */
static UA_INLINE void
UA_IJT_ProcessingTimesDataType_init(UA_IJT_ProcessingTimesDataType *p) {
    memset(p, 0, sizeof(UA_IJT_ProcessingTimesDataType));
}

static UA_INLINE UA_IJT_ProcessingTimesDataType *
UA_IJT_ProcessingTimesDataType_new(void) {
    return (UA_IJT_ProcessingTimesDataType*)UA_new(&UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE UA_StatusCode
UA_IJT_ProcessingTimesDataType_copy(const UA_IJT_ProcessingTimesDataType *src, UA_IJT_ProcessingTimesDataType *dst) {
    return UA_copy(src, dst, &UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE]);
}

UA_DEPRECATED static UA_INLINE void
UA_IJT_ProcessingTimesDataType_deleteMembers(UA_IJT_ProcessingTimesDataType *p) {
    UA_clear(p, &UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE void
UA_IJT_ProcessingTimesDataType_clear(UA_IJT_ProcessingTimesDataType *p) {
    UA_clear(p, &UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE]);
}

static UA_INLINE void
UA_IJT_ProcessingTimesDataType_delete(UA_IJT_ProcessingTimesDataType *p) {
    UA_delete(p, &UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE]);
}

#if defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
# pragma GCC diagnostic pop
#endif

_UA_END_DECLS

#endif /* TYPES_TIGHTENING_GENERATED_HANDLING_H_ */
