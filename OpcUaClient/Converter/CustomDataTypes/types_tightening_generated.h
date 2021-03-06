/**********************************
 * Autogenerated -- do not modify *
 **********************************/

#ifndef TYPES_TIGHTENING_GENERATED_H_
#define TYPES_TIGHTENING_GENERATED_H_

#ifdef UA_ENABLE_AMALGAMATION
#include "open62541.h"
#else
#include <open62541/types.h>
#include <open62541/types_generated.h>

#endif



_UA_BEGIN_DECLS


/**
 * Every type is assigned an index in an array containing the type descriptions.
 * These descriptions are used during type handling (copying, deletion,
 * binary encoding, ...). */
#define UA_TYPES_TIGHTENING_COUNT 1
extern UA_EXPORT UA_DataType UA_TYPES_TIGHTENING[UA_TYPES_TIGHTENING_COUNT];

/**
 * ProcessingTimesDataType
 * ^^^^^^^^^^^^^^^^^^^^^^^
 * This structure contains measured times that were generated during the execution of a joining process. These measured values provide information about the duration required by the various sub-functions. */
typedef struct {
    UA_DateTime startTime;
    UA_DateTime endTime;
    UA_Double *acquisitionDuration;
    UA_Double *processingDuration;
} UA_IJT_ProcessingTimesDataType;

#define UA_TYPES_TIGHTENING_PROCESSINGTIMESDATATYPE 0


_UA_END_DECLS

#endif /* TYPES_TIGHTENING_GENERATED_H_ */
