#include "tll_instructions.h"

const tll_op_info_t OP_INFO[] = {
    {
        OP_PASS,
        NO_OPERAND,
        "PASS"
    },
    {
        OP_PUSH,
        ONE_OPERAND,
        "PUSH"
    },
    {
        OP_POP,
        NO_OPERAND,
        "POP"
    },
    {
        OP_ADD,
        NO_OPERAND,
        "ADD"
    },
    {
        OP_SUB,
        NO_OPERAND,
        "SUB"
    },
    {
        OP_SHOW,
        NO_OPERAND,
        "SHOW"
    },
};

tll_op_code_t get_op_code(const char* token)
{
    return OP_ERROR;
}

