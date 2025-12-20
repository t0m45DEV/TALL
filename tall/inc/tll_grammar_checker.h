#ifndef _TLL_CONTROL_UNIT
#define _TLL_CONTROL_UNIT

#include "tll_instructions.h"

/**
 * Checks the grammar for every line on the given code, fetching and decoding every instruction, and saves it into the given list
 *
 * Returns the count of errors found, or -1 if the function could not start the checking on the file
 */
int check_grammar(char* code, const tll_op_t* bytecode[]);

#endif
