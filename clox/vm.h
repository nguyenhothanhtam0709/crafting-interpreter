#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
#include "chunk.h"

#define STACK_MAX 256

typedef struct
{
    Chunk *chunk;
    /**
     * Instruction pointer or program counter
     */
    uint8_t *ip;
    Value stack[STACK_MAX];
    /**
     * Stack pointer
     */
    Value *stackTop;
    /**
     * Global variables
     */
    Table globals;
    /**
     * Global string pool for `string interning`
     *
     * @see https://craftinginterpreters.com/hash-tables.html#string-interning
     */
    Table strings;
    /**
     * List of all objects stored in heap
     */
    Obj *objects;
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif