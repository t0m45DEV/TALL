#include "tll_standard_library.h"

#include "tll_value.h"
#include "tll_vm.h"

#include <time.h>

/**
 * Returns the time passed since the program started.
 */
static tll_value native_clock(int arg_count, tll_value* args);

void load_tll_std_lib(void)
{
    define_native_function("clock", 5, native_clock, 0, NULL);
}

static tll_value native_clock(int arg_count, tll_value* args)
{
    if (arg_count != 0 && args != NULL)
    {
        // Unreachable.
    }
    return AS_TLL_FLOAT((double) clock() / CLOCKS_PER_SEC);
}

