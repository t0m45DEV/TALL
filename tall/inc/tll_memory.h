#ifndef _TLL_MEMORY_H
#define _TLL_MEMORY_H

#include "tll_common.h"

/**
 * Returns a new larger capacity based on the given one.
 */
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

/**
 * Allocates a new block of memory for pointer given the current and the desired capacity.
 */
#define GROW_ARRAY(type, pointer, old_capacity, new_capacity) ((type*) reallocate(pointer, sizeof(type) * old_capacity, sizeof(type) * new_capacity))

/**
 * Frees the memory of size old_capacity being used by the given pointer.
 */
#define FREE_ARRAY(type, pointer, old_capacity) (reallocate(pointer, sizeof(type) * (old_capacity), 0))

/**
 * Returns a type pointer to a memory chunk of size count.
 */
#define ALLOCATE_ARRAY(type, count) ((type*) reallocate(NULL, 0, sizeof(type) * count))

/**
 * The function in charge of managing all the memory use of TALL.
 *
 * If old_capacity is 0, and new_capacity is not 0, then it returns a new allocated block of memory.
 *
 * If old_capacity is not 0, and new_capacity is 0, then it frees the memory allocated for pointer.
 *
 * If old_capacity is not 0, and new_capacity is smaller than old_capacity, then it shrinks the existing allocation for pointer.
 *
 * If old_capacity is not 0, and new_capacity is larger than old_capacity, then it grows the existing allocation for pointer.
 */
void* reallocate(void* pointer, size_t old_capacity, size_t new_capacity);

#endif
