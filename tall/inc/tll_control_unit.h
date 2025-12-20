#ifndef _TLL_CONTROL_UNIT
#define _TLL_CONTROL_UNIT

#include "tll_instructions.h"

/**
 * Checks the grammar for every line on the given code, fetching and decoding every instruction, and saves it into the given list
 *
 * Returns the count of errors found, or -1 if the function could not start the checking on the file
 */
int check_grammar(char* code);

/**
 * Given a line of code, returns the equivalent instruction parsed a new string (without comments, spaces, etc)
 */
char* fetch_op(char* bytecode_line);

/**
 * Converts the given instruction in format "INT OP", where INT is the operation and OP is the operand, into the struct tll_op_t
 */
const tll_op_t decode_op(const char* line);

#endif
