#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"
#include "value.h"
#include "chunk.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * A stack frame
 */
typedef struct
{
    /**
     * The function being called
     */
    ObjClosure *closure;
    /**
     * Caller's current instruction pointer. When we return from a function, the VM will jump to the ip of the caller’s CallFrame and resume from there.
     */
    uint8_t *ip;
    /**
     * Local stack pointer, or the first slot of this stack frame in the stack
     */
    Value *slots;
} CallFrame;

typedef struct
{
    /**
     * Call stack
     */
    CallFrame frames[FRAMES_MAX];
    /**
     * Stack frame count
     */
    int frameCount;
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

    ObjUpvalue *openUpvalues;
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