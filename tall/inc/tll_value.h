#ifndef _TLL_VALUE_H
#define _TLL_VALUE_H

#include "tll_common.h"

#define AS_TLL_BOOL(value)   ((tll_value) {VAL_BOOL,  {.tll_bool  = value}})
#define AS_TLL_NULL          ((tll_value) {VAL_NULL,  {.tll_int   = 0}})
#define AS_TLL_INT(value)    ((tll_value) {VAL_INT,   {.tll_int   = value}})
#define AS_TLL_FLOAT(value)  ((tll_value) {VAL_FLOAT, {.tll_float = value}})

#define AS_C_BOOL(value)  ((value).as.tll_bool)
#define AS_C_INT(value)   ((value).as.tll_int)
#define AS_C_FLOAT(value) ((value).as.tll_float)

#define IS_BOOL(value)  ((value).type == VAL_BOOL)
#define IS_NULL(value)  ((value).type == VAL_NULL)
#define IS_INT(value)   ((value).type == VAL_INT)
#define IS_FLOAT(value) ((value).type == VAL_FLOAT)

#define IS_NUMBER(value) ((value).type == VAL_INT || (value).type == VAL_FLOAT)

/**
 * The posible types for the data on TALL.
 */
typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
} tll_value_type;

/**
 * This the representation in C for a TALL value.
 */
typedef struct {
    tll_value_type type;
    union {
        bool tll_bool;
        int tll_int;
        double tll_float;
    } as;
} tll_value;

/**
 * A collection of TALL values, in the form of a dynamic array.
 */
typedef struct {
    int capacity;
    int count;
    tll_value* values;
} tll_value_array;

/**
 * Initialize the given value array.
 */
void init_value_array(tll_value_array* value_array);

/**
 * Frees the memory allocated by the given value array.
 */
void free_value_array(tll_value_array* value_array);

/**
 * Writes the given value into the given value array.
 */
void write_value_array(tll_value_array* value_array, tll_value value);

/**
 * Returns true if the two given values are equal.
 */
bool are_equals(tll_value value1, tll_value value2);

/**
 * Prints out to the standard output the given value.
 */
void print_value(tll_value value);

/**
 * Prints out to the standard output the type of the given value.
 */
void print_type(tll_value value);

#endif
