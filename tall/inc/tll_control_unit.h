#include "tll_instructions.h"

/**
 * Represents an operation, with an operation code and an operand
 *
 * If the instruction given by OP_CODE does not have an operand, the value inside OPERAND should be ignored
 */
typedef struct {
    const TLL_OP_CODE OP_CODE; // The code of the operation, check TLL_OP_CODE for more info
    const float OPERAND;       // The operand of the instruction, if the operation has an operand to start with
} tll_op_t;

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
const tll_op_t decode_op(char* token);

/**
 * Does what the given operation tells it to do
 *
 * Returns 0 if the action of the operation could be made
 *
 * Returns 1 if the OP_CODE of the given operation is ERROR_OP, or if the action given by OP_CODE could not be made
 */
int execute_op(const tll_op_t op);


