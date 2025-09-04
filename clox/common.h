#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

/**
 * Common field for dynamic array struct
 *
 * @var count
 * @var capacity
 */
#define DYNAMIC_ARRAY_STRUCT_COMMON_FIELD \
    int count;                            \
    int capacity;

#define INIT_DYNAMIC_ARRAY_STRUCT_COMMON_FIELD(pointer) \
    pointer->count = 0;                                 \
    pointer->capacity = 0;

#endif