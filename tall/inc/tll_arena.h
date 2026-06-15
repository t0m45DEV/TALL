#ifndef _TLL_ARENA_H
#define _TLL_ARENA_H

#include <stdint.h>
#include <stdio.h>

/**
 * A memory chunk, like a big pool for data collection and fast creation.
 */
typedef struct tll_memory_arena {
    uint8_t* capacity;
    uint8_t* available;
    uint8_t* space;
    struct tll_memory_arena* next;
} tll_memory_arena;

/**
 * Allocates the memory needed for a memory arena.
 */
tll_memory_arena* create_arena(void);

/**
 * Frees the memory used by the given arena.
 */
void free_arena(tll_memory_arena* arena);

/**
 * Adds the given element to the given memory arena and returns that arena pointer, unless there is no more space available in there.
 *
 * If there is no more space, it creates a new arena next to the given one, and returns the pointer to the new arena.
 */
tll_memory_arena* add_to_arena(tll_memory_arena* arena, const void* element, size_t size);

#endif
