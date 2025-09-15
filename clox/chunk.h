#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

/**
 * In this bytecode format, each instruction has a one-byte operation code (universally shortened to opcode).
 */

typedef enum
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL, // define global variable
    OP_SET_GLOBAL,
    OP_GET_UPVALUE, // resolve upvalue for a closure
    OP_SET_UPVALUE, // resolve upvalue for a closure
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,         // print statement
    OP_JUMP,          // unconditional jump
    OP_JUMP_IF_FALSE, // jump by offset if value is falsey
    OP_LOOP,          // loop, actually it is a backward jump
    OP_CALL,          // invoke function
    OP_INVOKE, 
    OP_CLOSURE,       // define closure
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_METHOD, // define method for a class
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