#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

typedef enum
{
    OBJ_FUNCTION,
    OBJ_NATIVE, // native function
    OBJ_STRING
} ObjType;

/**
 * Lox value whose state lives on the heap is an Obj.
 */
struct Obj
{
    ObjType type;
    struct Obj *next;
};

/**
 * Lox function
 */
typedef struct
{
    Obj obj;
    /**
     * Number of parameters the function expects
     */
    int arity;
    /**
     * Point to the first bytecode of the function
     */
    Chunk chunk;
    /**
     * Function name
     */
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

/**
 * Object presents a native function
 */
typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

/**
 * Lox string
 */
struct ObjString
{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

ObjFunction *newFunction();
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length);
void printObj(Value value);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif