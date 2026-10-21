#include "tll_standard_library.h"

#include "tll_value.h"
#include "tll_object.h"
#include "tll_vm.h"

#include <stdio.h>
#include <time.h>

/**
 * Returns the time passed since the program started.
 */
static tll_value native_clock(int arg_count, const tll_value* args);

/**
 * Prints out to standard output the given value.
 */
static tll_value native_print(int arg_count, const tll_value* args);

/**
 * Returns the given null value as a string value.
 */
static tll_value native_null_to_str(int arg_count, const tll_value* args);

/**
 * Returns the given bool value as a string value.
 */
static tll_value native_bool_to_str(int arg_count, const tll_value* args);

/**
 * Returns the given int value as a string value.
 */
static tll_value native_int_to_str(int arg_count, const tll_value* args);

/**
 * Returns the given float value as a string value.
 */
static tll_value native_float_to_str(int arg_count, const tll_value* args);

void load_tll_std_lib(void)
{
    define_native_function("clock", 5, native_clock, 0, NULL);
    define_native_function("print", 5, native_print, 1, (const tll_value[1]) {AS_TLL_OBJ(&TLL_TYPE_STRING)});

    define_native_method(VAL_NULL, "to_str", 6, native_null_to_str, 1, (const tll_value[1]) {AS_TLL_NULL});
    define_native_method(VAL_BOOL, "to_str", 6, native_bool_to_str, 1, (const tll_value[1]) {AS_TLL_BOOL(true)});
    define_native_method(VAL_INT, "to_str", 6, native_int_to_str, 1, (const tll_value[1]) {AS_TLL_INT(0)});
    define_native_method(VAL_FLOAT, "to_str", 6, native_float_to_str, 1, (const tll_value[1]) {AS_TLL_FLOAT(0.0)});
}

static tll_value native_clock(int arg_count, const tll_value* args)
{
    if (arg_count != 0 && args != NULL)
    {
        // Unreachable.
    }
    return AS_TLL_FLOAT((double) clock() / CLOCKS_PER_SEC);
}

static tll_value native_print(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    print_value(*args);
    printf("\n");
    return AS_TLL_NULL;
}

static tll_value native_null_to_str(int arg_count, const tll_value* args)
{
    (void) arg_count;
    (void) args;

    return AS_TLL_OBJ(copy_string("null", 4));
}

static tll_value native_bool_to_str(int arg_count, const tll_value* args)
{
    (void) arg_count;

    tll_value ret;

    if (AS_C_BOOL(args[0]))
    {
        ret = AS_TLL_OBJ(copy_string("true", 4));
    }
    else
    {
        ret = AS_TLL_OBJ(copy_string("false", 5));
    }
    return ret;
}

static tll_value native_int_to_str(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    char buffer[32];

    int len = snprintf(buffer, sizeof(buffer), "%d", AS_C_INT(args[0]));

    return AS_TLL_OBJ(copy_string(buffer, len));
}

static tll_value native_float_to_str(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    char buffer[64];

    int len = snprintf(buffer, sizeof(buffer), "%g", AS_C_FLOAT(args[0]));

    return AS_TLL_OBJ(copy_string(buffer, len));
}

