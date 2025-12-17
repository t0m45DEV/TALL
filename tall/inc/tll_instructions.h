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

typedef struct {
    const tll_operand_cnt_t OPERANDS_NEEDED;
    const char* NAME;
} tll_op_info_t;

/**
 * Returns the operation code corresponding to the given token
 *
 * @note The token is asumed with the form of "INSTRUCTION\0"
 */
tll_op_code_t get_op_code(const char* token);

#endif
