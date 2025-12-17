#include "tll_instructions.h"

const tll_op_info_t OP_INFO[] = {
    {
        NO_OPERAND,
        "PASS"
    },
    {
        ONE_OPERAND,
        "PUSH"
    },
    {
        NO_OPERAND,
        "POP"
    },
    {
        NO_OPERAND,
        "ADD"
    },
    {
        NO_OPERAND,
        "SUB"
    },
    {
        NO_OPERAND,
        "SHOW"
    },
};

tll_op_code_t get_op_code(const char* token)
{
    return ERROR_OP;
}

