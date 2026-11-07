#include "tll_standard_library.h"

#include "tll_value.h"
#include "tll_object.h"
#include "tll_vm.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
 * Prints out to standard output the given formated string.
 */
static tll_value native_printf(int arg_count, const tll_value* args);

/**
 * Halts the program waiting for user input in the form of a string of the given size.
 *
 * Any remaining characters on that line are discarded.
 */
static tll_value native_input(int arg_count, const tll_value* args);

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

/**
 * Returns the given string value as an int value.
 */
static tll_value native_str_to_int(int arg_count, const tll_value* args);

void load_tll_std_lib(void)
{
    define_native_function("clock", 5, native_clock, 0, NULL);
    define_native_function("print", 5, native_print, 1, (const tll_value[1]) {AS_TLL_OBJ(&TLL_TYPE_STRING)});
    define_native_function("printf", 6, native_printf, 1, (const tll_value[1]) {AS_TLL_OBJ(&TLL_TYPE_STRING)});
    define_native_function("input", 5, native_input, 1, (const tll_value[1]) {AS_TLL_INT(0)});

    define_native_method(VAL_NULL, "to_str", 6, native_null_to_str, 1, (const tll_value[1]) {AS_TLL_NULL});
    define_native_method(VAL_BOOL, "to_str", 6, native_bool_to_str, 1, (const tll_value[1]) {AS_TLL_BOOL(true)});
    define_native_method(VAL_INT, "to_str", 6, native_int_to_str, 1, (const tll_value[1]) {AS_TLL_INT(0)});
    define_native_method(VAL_FLOAT, "to_str", 6, native_float_to_str, 1, (const tll_value[1]) {AS_TLL_FLOAT(0.0)});

    define_native_method(VAL_OBJ, "to_int", 6, native_str_to_int, 1, (const tll_value[1]) {AS_TLL_OBJ(&TLL_TYPE_STRING)});
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

static tll_value native_printf(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    tll_string* input = AS_TLL_STRING(*args);

    char formated[input->length + 1];

    int i = 0;

    for (int j = 0; j < input->length; j++)
    {
        if (input->chars[j] == '\\' && j + 1 < input->length)
        {
            switch (input->chars[j + 1])
            {
                case '\\':
                {
                    formated[i] = '\\';
                    i++;
                    break;
                }
                case '\"':
                {
                    formated[i] = '\"';
                    i++;
                    break;
                }
                case 'n':
                {
                    formated[i] = '\n';
                    i++;
                    break;
                }
                case 't':
                {
                    formated[i] = '\t';
                    i++;
                    break;
                }
                default:
                {
                    // Runtime error.
                    catch_native_error("There is no escape sequence for the symbol \\%c.", input->chars[j + 1]);
                    return AS_TLL_NULL;
                    break;
                }
            }
            j++;
        }
        else
        {
            formated[i] = input->chars[j];
            i++;
        }
    }
    printf("%.*s", i, formated);
    return AS_TLL_NULL;
}

static tll_value native_input(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    int char_count = AS_C_INT(*args) + 1;

    if (char_count <= 0)
    {
        // Runtime error.
        catch_native_error("Can't create an string with negative length.");
        return AS_TLL_OBJ(copy_string("", 0));
    }
    char str[char_count];
    str[0] = '\0';

    if (fgets(str, char_count, stdin) == NULL)
    {
        // Runtime error.
        catch_native_error("There was an error reading from input.");
        return AS_TLL_OBJ(copy_string("", 0));
    }
    size_t len = strnlen(str, (size_t) char_count);

    if (len > 0  && str[len - 1] == '\n')
    {
        len = len - 1;
    }
    else
    {
        // The line was longer than the buffer.
        // We need to discard the leftover content for next input() calls.
        char c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
            // Discard.
        }
    }
    tll_value ret = AS_TLL_OBJ(copy_string(str, len));

    return ret;
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

static tll_value native_str_to_int(int arg_count, const tll_value* args)
{
    if (arg_count != 1)
    {
        // Unreachable.
    }
    return AS_TLL_INT(atoi(AS_TLL_STRING(*args)->chars));
}

