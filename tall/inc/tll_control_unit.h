#ifndef _TLL_CONTROL_UNIT
#define _TLL_CONTROL_UNIT

#include "tll_instructions.h"

/**
 * Process the entire code, fetching and decoding every instruction
 *
 * Returns 0 if success processing the entire string, and returns 1 otherwise
 */
int process_code(char* code);

/**
 * Given a line of code, returns the equivalent instruction parsed a new string (without comments, spaces, etc)
 */
char* fetch_op(char* bytecode_line);

/**
 * Converts the given instruction in format "INT OP", where INT is the operation and OP is the operand, into the struct tll_op_t
 */
const tll_op_t decode_op(const char* line);

/**
 * Does what the given operation tells it to do
 *
 * Returns 0 if the action of the operation could be made
 *
 * Returns 1 if the OP_CODE of the given operation is ERROR_OP, or if the action given by OP_CODE could not be made
 */
int execute_op(const tll_op_t op);

#endif
