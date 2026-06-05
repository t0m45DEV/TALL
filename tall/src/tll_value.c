#include "tll_value.h"
#include "tll_memory.h"

#include <stdio.h>

void init_value_array(tll_value_array* value_array)
{
    value_array->capacity = 0;
    value_array->count = 0;
    value_array->values = NULL;
}

void free_value_array(tll_value_array* value_array)
{
    FREE_ARRAY(tll_value, value_array->values, value_array->capacity);
    init_value_array(value_array);
}

void write_value_array(tll_value_array *value_array, tll_value value)
{
    if (value_array->count + 1 > value_array->capacity)
    {
        int old_capacity = value_array->capacity;
        value_array->capacity = GROW_CAPACITY(old_capacity);
        value_array->values = GROW_ARRAY(tll_value, value_array->values, old_capacity, value_array->capacity);
    }
    value_array->values[value_array->count] = value;
    value_array->count++;
}

void print_value(tll_value value)
{
    printf("%g", value);
}

