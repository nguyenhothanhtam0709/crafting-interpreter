#include <stdio.h>

#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"

VM vm;

static InterpretResult run();
static void resetStack();

void initVM()
{
    resetStack();
}

void freeVM() {}

/**
 * Execute the given chunk of instructions
 */
InterpretResult interpret(const char *source)
{
    compile(source);
    return INTERPRET_OK;
}

void push(Value value)
{
    *(vm.stackTop) = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *(vm.stackTop);
}

/**
 * Execute instructions stored in VM
 */
static InterpretResult run()
{
/**
 * Return current instruction and advance instruction pointer
 */
#define READ_BYTE() (*(vm.ip++))
/**
 * Read the next byte from the bytecode, treat the resulting number as an index,
 * and look up the corresponding Value in the chunk’s constant table.
 */
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
/**
 * Handle binary operators
 */
#define BINARY_OP(op)     \
    do                    \
    {                     \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

    for (;;)
    {
        /**
         * Other technique to implement bytecode dispatching:
         * - Direct threaded code
         * - Jump table
         * - Computed goto
         */

#ifdef DEBUG_TRACE_EXECUTION
        printf("            ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_ADD:
        {
            BINARY_OP(+);
            break;
        }
        case OP_SUBTRACT:
        {
            BINARY_OP(-);
            break;
        }
        case OP_MULTIPLY:
        {
            BINARY_OP(*);
            break;
        }
        case OP_DIVIDE:
        {
            BINARY_OP(/);
            break;
        }
        case OP_NEGATE:
        {
            push(-pop());
            break;
        }
        case OP_RETURN:
        {
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

static void resetStack()
{
    vm.stackTop = vm.stack;
}