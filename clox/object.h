#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value) isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

typedef enum
{
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE, // native function
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

/**
 * Lox value whose state lives on the heap is an Obj.
 */
struct Obj
{
    ObjType type;
    bool isMarked;
    struct Obj *next;
};

/**
 * Runtime representation for upvalues
 */
typedef struct ObjUpvalue
{
    Obj obj;
    Value *location;
    /**
     * Save closure variable after it is popped out of stack frame
     */
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

/**
 * Lox function, all functions are wrapped in ObjClosure
 */
typedef struct
{
    Obj obj;
    /**
     * Number of parameters the function expects
     */
    int arity;
    /**
     * Number of upvalue of this function
     */
    int upvalueCount;
    /**
     * Point to the first bytecode of the function
     */
    Chunk chunk;
    /**
     * Function name
     */
    ObjString *name;
} ObjFunction;

typedef struct
{
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct
{
    Obj obj;
    ObjString *name;
} ObjClass;

typedef struct
{
    Obj obj;
    ObjClass *klass;
    /** @brief Store fields */
    Table fields;
} ObjInstance;

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

ObjClass *newClass(ObjString *name);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
ObjInstance *newInstance(ObjClass *klass);
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length);
ObjUpvalue *newUpvalue(Value *slot);
void printObj(Value value);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif