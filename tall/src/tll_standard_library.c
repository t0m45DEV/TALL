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

void load_tll_std_lib(void)
{
    define_native_function("clock", 5, native_clock, 0, NULL);
    define_native_function("print", 5, native_print, 1, (const tll_value[1]) {AS_TLL_FLOAT(0)});
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
    // TODO: Print only strings.
    print_value(*args);
    printf("\n");
    return AS_TLL_NULL;
}

