#ifndef _TLL_OBJECT_H
#define _TLL_OBJECT_H

#include "tll_common.h"
#include "tll_value.h"
#include "tll_code_chunk.h"

#define OBJ_TYPE(value) (AS_C_OBJ(value)->type)

#define IS_STRING(value)   (is_obj_type((value), OBJ_STRING))
#define IS_FUNCTION(value) (is_obj_type((value), OBJ_FUNCTION))

#define AS_TLL_STRING(value)  ((tll_string*) AS_C_OBJ(value))
#define AS_C_STRING(value) (((tll_string*) AS_C_OBJ(value))->chars)

#define AS_TLL_FUNCTION(value) ((tll_function*) AS_C_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
} tll_obj_type;

struct tll_obj {
    tll_obj_type type;
};

struct tll_string {
    tll_obj obj;
    int length;
    char* chars;
};

struct tll_function {
    tll_obj obj;
    int arity;
    tll_value_type* arguments_types;
    tll_value_type return_type;
    tll_code_chunk code_chunk;
    tll_string* name;
};

static inline bool is_obj_type(tll_value value, tll_obj_type type)
{
    return IS_OBJ(value) && (OBJ_TYPE(value) == type);
}

/**
 * Returns a TLL string object using directly the given character pointer of size length.
 */
tll_string* take_string(char* chars, int length);

/**
 * Creates a TLL string object based on the given array of characters with size length.
 */
tll_string* copy_string(const char* chars, int length);

/**
 * Creates a TLL function object.
 */
tll_function* new_function(void);

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
