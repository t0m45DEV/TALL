#ifndef _TLL_INSTRUCTIONS
#define _TLL_INSTRUCTIONS

typedef enum {
    ERROR_OP = -1,
    PASS,
    PUSH,
    POP,
    ADD,
    SUB,
    SHOW,

    ALL_OP_CODES
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
