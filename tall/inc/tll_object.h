#ifndef _TLL_OBJECT_H
#define _TLL_OBJECT_H

#include "tll_common.h"
#include "tll_value.h"
#include "tll_code_chunk.h"

#include <stdint.h>

#define OBJ_TYPE(value) (AS_C_OBJ(value)->type)

#define IS_STRING(value)   (is_obj_type((value), OBJ_STRING))
#define IS_FUNCTION(value) (is_obj_type((value), OBJ_FUNCTION))
#define IS_NATIVE(value)   (is_obj_type((value), OBJ_NATIVE))

#define AS_TLL_STRING(value)  ((tll_string*) AS_C_OBJ(value))
#define AS_C_STRING(value) (((tll_string*) AS_C_OBJ(value))->chars)

#define AS_TLL_FUNCTION(value) ((tll_function*) AS_C_OBJ(value))

#define AS_TLL_NATIVE(value) ((tll_native_function*) AS_C_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
} tll_obj_type;

struct tll_obj {
    tll_obj_type type;
};

struct tll_string {
    tll_obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

struct tll_function {
    tll_obj obj;
    int arity;
    tll_value_type* arguments_types;
    tll_value_type return_type;
    tll_code_chunk code_chunk;
    tll_string* name;
};

typedef tll_value (*native_function)(int arg_count, const tll_value* args);

struct tll_native_function {
    tll_obj obj;
    native_function function;
    int parameter_count;
    tll_value* parameters;
};

static inline bool is_obj_type(tll_value value, tll_obj_type type)
{
    return IS_OBJ(value) && (OBJ_TYPE(value) == type);
}

/**
 * A type-only string value, used purely as a type tag when needed.
 */
extern const tll_string TLL_TYPE_STRING;

/**
 * Returns a TLL string object using directly the given character pointer of size length.
 */
tll_string* take_string(char* chars, int length);

/**
 * Creates a TLL string object based on the given array of characters with size length.
 */
tll_string* copy_string(const char* chars, int length);

/**
 * Returns the hash to the given key.
 */
uint32_t get_hash(const char* key, int length);

/**
 * Creates a TLL function object.
 */
tll_function* new_function(void);

/**
 * Creates a TLL native function object.
 */
tll_native_function* new_native_function(native_function function, int parameter_count, const tll_value parameters[]);

/**
 * Checks the given values types with the parameters types for the given native function.
 */
bool check_types(const tll_native_function* native, int arg_count, const tll_value* args);

/**
 * Frees the memory used by the given object.
 */
void free_object(tll_obj* object);

/**
 * Prints out the contents of the given value as an object.
 */
void print_object(tll_value value);

/**
 * Ininitalize the pool for dinamyc allocated TLL objects.
 */
void init_object_pool(void);

/**
 * Frees the memory used by the TLL objects pool.
 */
void free_object_pool(void);

#endif
