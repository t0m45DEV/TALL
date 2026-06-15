#include "tll_arena.h"

#include "tll_memory.h"

#include <string.h>

// The size of a given arena.
#define ARENA_SIZE (1024 * 1024)

tll_memory_arena* create_arena(void)
{
    tll_memory_arena* arena = reallocate(NULL, 0, sizeof(tll_memory_arena));

    arena->next = NULL;
    arena->space = reallocate(NULL, 0, ARENA_SIZE);
    arena->available = arena->space;
    arena->capacity = arena->space + ARENA_SIZE;

    return arena;
}

void free_arena(tll_memory_arena* arena)
{
    tll_memory_arena* current = arena;

    while (current != NULL)
    {
        tll_memory_arena* next = current->next;

        current->capacity = 0;
        current->next = NULL;
        current->available = NULL;
        current->space = reallocate(current->space, ARENA_SIZE, 0);

        reallocate(current, sizeof(tll_memory_arena), 0);
        current = next;
    }
}

tll_memory_arena* add_to_arena(tll_memory_arena* arena, const void* element, size_t size)
{
    tll_memory_arena* ret = arena;

    if (arena->available + size > arena->capacity)
    {
        arena->next = create_arena();
        ret = arena->next;
    }
    memcpy(ret->available, element, size);
    ret->available += size;

    return ret;
}

