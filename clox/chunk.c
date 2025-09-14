#include "chunk.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk *chunk)
{
    INIT_DYNAMIC_ARRAY_STRUCT_COMMON_FIELD(chunk)
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&(chunk->constants));
}

void freeChunk(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&(chunk->constants));
    initChunk(chunk);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(Chunk *chunk, Value value)
{
    push(value); // push value to stack to prevent it from being garbage collected when chunk->constants is being resized (re-allocated).
    writeValueArray(&(chunk->constants), value);
    pop();
    return chunk->constants.count - 1;
}