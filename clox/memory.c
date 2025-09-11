#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, newSize);
    if (result == NULL)
    {
        exit(EXIT_FAILURE);
    }

    return result;
}

static void freeObject(Obj *object)
{
    switch (object->type)
    {
    case OBJ_CLOSURE:
    {
        /**
         * @details We free only the ObjClosure itself, not the ObjFunction.
         * That’s because the closure doesn’t own the function.
         * There may be multiple closures that all reference the same function,
         * and none of them claims any special privilege over it.
         * We can’t free the ObjFunction until all objects referencing it are gone—including
         * even the surrounding function whose constant table contains it.
         * Tracking that sounds tricky, and it is!
         * That’s why we’ll write a garbage collector soon to manage it for us.
         */

        ObjClosure *closure = (ObjClosure *)object;
        FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
        FREE(ObjClosure, object);
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        freeChunk(&function->chunk);
        FREE(ObjFunction, object);
        break;
    }
    case OBJ_NATIVE:
    {
        FREE(ObjNative, object);
        break;
    }
    case OBJ_STRING:
    {
        ObjString *string = (ObjString *)object;
        FREE_ARRAY(char, string->chars, string->length + 1);
        FREE(ObjString, object);
        break;
    }
    case OBJ_UPVALUE:
    {
        FREE(ObjUpvalue, object);
        break;
    }
    }
}

void freeObjects()
{
    Obj *object = vm.objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }
}
