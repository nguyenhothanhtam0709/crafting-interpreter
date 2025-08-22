#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/**
 * This typedef abstracts how Lox values are concretely represented in C.
 */
typedef double Value;

/**
 * Constant pool
 *
 * @details constant pool is an array of values. The instruction to load a constant looks up the value by index in that array.
 */
typedef struct
{
    DYNAMIC_ARRAY_STRUCT_COMMON_FIELD

    Value *values;
} ValueArray;

void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif