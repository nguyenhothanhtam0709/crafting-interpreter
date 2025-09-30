#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// @brief Enable nan boxing
#define NAN_BOXING
/**
 * `Stress test” mode for the garbage collector. When this flag is defined, the GC runs as often as it possibly can.
 */
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define UINT8_COUNT (UINT8_MAX + 1)

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