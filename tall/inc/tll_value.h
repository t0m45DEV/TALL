#ifndef _TLL_VALUE_H
#define _TLL_VALUE_H

#include "tll_common.h"

/**
 * This the representation in C for a TALL value.
 */
typedef double tll_value;

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
 * Prints out to the standard output the given value.
 */
void print_value(tll_value value);

#endif
