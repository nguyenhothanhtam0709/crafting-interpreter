#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "memory.h"
#include "value.h"

/**
 * In this bytecode format, each instruction has a one-byte operation code (universally shortened to opcode).
 */

typedef enum
{
    OP_CONSTANT,
    OP_RETURN
} OpCode;

/**
 * Sequences of bytecode
 */
typedef struct
{
    DYNAMIC_ARRAY_STRUCT_COMMON_FIELD

    uint8_t *code;
    int *lines;
    /**
     * Constant pool
     */
    ValueArray constants;

} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);

#endif