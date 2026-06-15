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

bool are_equals(tll_value value1, tll_value value2)
{
    if (IS_BOOL(value1) && IS_BOOL(value2))
    {
        return AS_C_BOOL(value1) == AS_C_BOOL(value2);
    }
    else if (IS_INT(value1) && IS_INT(value2))
    {
        return AS_C_INT(value1) == AS_C_INT(value2);
    }
    else if (IS_FLOAT(value1) && IS_FLOAT(value2))
    {
        return AS_C_FLOAT(value1) == AS_C_FLOAT(value2);
    }
    return false; // Unreachable.
}

void print_value(tll_value value)
{
    switch (value.type)
    {
        case VAL_NULL:
            printf("null");
            break;

        case VAL_BOOL:
            if (AS_C_BOOL(value))
            {
                printf("true");
            }
            else
            {
                printf("false");
            }
            break;

        case VAL_INT:
            printf("%i", AS_C_INT(value));
            break;

        case VAL_FLOAT:
            printf("%g", AS_C_FLOAT(value));
            break;
    }
}

void print_type(tll_value value)
{
    switch (value.type)
    {
        case VAL_NULL:
            printf("null");
            break;

        case VAL_BOOL:
            printf("bool");
            break;

        case VAL_INT:
            printf("int");
            break;

        case VAL_FLOAT:
            printf("float");
            break;
    }
}

