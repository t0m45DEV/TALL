#ifndef _TLL_INSTRUCTIONS
#define _TLL_INSTRUCTIONS

typedef enum {
    OP_ERROR = -1,
    OP_PASS,
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_SHOW,

    OP_CODES_COUNT
} tll_op_code_t;

typedef enum {
    NO_OPERAND = 0,
    ONE_OPERAND
} tll_operand_cnt_t;

/**
 * Represents an operation, with an operation code and an operand
 *
 * If the instruction given by OP_CODE does not have an operand, the value inside OPERAND should be ignored
 */
typedef struct {
    const tll_op_code_t OP_CODE;
    const tll_operand_cnt_t OPERANDS_NEEDED;
    const char* TEXT;
} tll_op_t;

/**
 * Initialize the necesary structures so we can have quick instruction searchs
 *
 * Returns 0 if everything went well, returns 1 otherwise
 */
int init_instructions_set(void);

/**
 * Frees the memory of the quick search structures used
 */
void destroy_instructions_set(void);

/**
 * Returns the operation code corresponding to the given token
 *
 * @note The token is asumed with the form of "INSTRUCTION\0"
 */
tll_op_t get_op_code(const char* token);

#endif
