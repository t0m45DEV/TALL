#include "tll_memory.h"

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
        printf("[FATAL ERROR] Tried to allocate %lu bytes of memory and failed.\n", new_capacity);
        exit(EXIT_FAILURE);
    }
    return result;
}

