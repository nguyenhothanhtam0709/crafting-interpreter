#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

/** Minimum threshold of array capacity*/
#define ARRAY_COUNT_MINIMUM_THRESHOLD 8
/** Scale factor of array capacity */
#define ARRAY_COUNT_SCALE_FACTOR 2

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

/**
 * Calculate new capacity for dynamic arrays based on a given current capacity.
 */
#define GROW_CAPACITY(capacity)                 \
    ((capacity) < ARRAY_COUNT_MINIMUM_THRESHOLD \
         ? ARRAY_COUNT_MINIMUM_THRESHOLD        \
         : (capacity) * ARRAY_COUNT_SCALE_FACTOR)

/**
 * Re-allocate new memory region for dynamic arrays
 *
 * @param type type of array's element
 * @param pointer pointer of array
 * @param oldCount array's old capacity
 * @param newCount array's new capacity
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))
/**
 * Free dynamic arrays
 *
 * @param type type of array's element
 * @param pointer pointer of array
 * @param oldCount array's current capacity
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * Dynamic memory allocation
 *
 * @details The two size arguments control which operation to perform:
 *
 * oldSize   | newSize     | Operation
 * --------- | ----------- | --------------------------------------
 * 0         | Non-zero    | Allocate new block.
 * Non-zero  | 0           | Free allocation.
 * Non-zero  | < oldSize   | Shrink existing allocation.
 * Non-zero  | > oldSize   | Grow existing allocation.
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif