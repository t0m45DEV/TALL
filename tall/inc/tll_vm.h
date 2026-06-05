#ifndef _TLL_VM_H
#define _TLL_VM_H

#include "tll_code_chunk.h"
#include "tll_value.h"

/**
 * The C representation for the TALL VM.
 */
typedef struct {
    tll_code_chunk* code_chunk;
    uint8_t* ip;
    int stack_capacity;
    tll_value* stack;
    tll_value* stack_top;
} tll_vm;

/**
 * The possible results for the TALL VM.
 */
typedef enum {
    TLL_INTERPRET_OK,
    TLL_INTERPRET_COMPILE_ERROR,
    TLL_INTERPRET_RUNTIME_ERROR,
} tll_interpret_result;

/**
 * Initialize the TALL VM.
 */
void init_VM(void);

/**
 * Frees all the memory needed for the TALL VM.
 */
void free_VM(void);

/**
 * Compiles and executes the given code_chunk and returns it's result in the form of an enum.
 */
tll_interpret_result interpret_code(const char* source_code);

/**
 * Saves the given value to the TALL VM stack.
 */
void push(tll_value value);

/**
 * Returns the current value at the top of the TALL VM stack, and delets it.
 */
tll_value pop(void);

#endif
