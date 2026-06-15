#include "tll_memory.h"

#include <stdint.h>
#include <string.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>

void* reallocate(void* pointer, size_t old_capacity, size_t new_capacity)
{
    if (new_capacity == 0)
    {
        free(pointer);
        return NULL;
    }
    void* result = realloc(pointer, new_capacity);

    if (result == NULL)
    {
        printf("[FATAL ERROR] Tried to allocate %zu bytes of memory and failed.\n", new_capacity);
        exit(EX_OSERR);
    }

    if (new_capacity > old_capacity)
    {
        memset((uint8_t*) result + old_capacity, 0, new_capacity - old_capacity);
    }
    return result;
}

