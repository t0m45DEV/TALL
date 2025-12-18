#ifndef _TLL_STACK
#define _TLL_STACK

/**
 * Saves the given element into the head of the stack
 *
 * Returns 0 if the stack is not full before adding the number, and 1 otherwise
 */
int stack_push(float element);

/**
 * Saves the number at the head of the stack into the given element, and deletes it from the stack
 *
 * Returns 0 if stack is not empty before deleting the number, and 1 otherwise
 */
int stack_pop(float* element);

/**
 * Saves the number at the i-th position in the stack into the given pointer
 *
 * Returns 0 if i is on range, and 1 otherwise
 */
int stack_peek(float* element, int i);

/**
 * Returns the current size of the stack being used
 */
int stack_size(void);

/**
 * Resets the stack back to the initial state
 */
void stack_clear(void);

#endif
