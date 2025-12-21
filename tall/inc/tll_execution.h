#ifndef _TLL_EXECUTION
#define _TLL_EXECUTION

#include "tll_instructions.h"

/**
 * Executes the given bytecode starting from position 0
 *
 * Returns 0 if the execution was succesfull, returns 1 otherwise
 */
int execute(const tll_op_t* bytecode[], int size);

#endif
