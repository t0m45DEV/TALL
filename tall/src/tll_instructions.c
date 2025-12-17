#include "tll_instructions.h"

#include "tll_dictionary.h"
#include "tll_log.h"

#include <stdlib.h>

tll_dictionary_t* instructions_by_text;

const tll_op_t OP_INFO[] = {
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

int init_instructions_set(void)
{
    log_info("Setting up instruction set...");
    instructions_by_text = create_dictionary();

    if (!instructions_by_text)
    {
        log_error("Dictionary for instructions set failed!");
        return 1;
    }

    for (tll_op_code_t i = 0; i < OP_CODES_COUNT; i++)
    {
        log_info("Adding element %i with key %s", i, OP_INFO[i].TEXT);

        int status = add_item(instructions_by_text, OP_INFO[i].TEXT, (void*) &OP_INFO[i]);
        log_info("Status %i", status);

        if (status != 0)
        {
            log_error("There is hashing collision between %s and %s",
                      OP_INFO[i].TEXT,
                      ((tll_op_t*) get_item(instructions_by_text, OP_INFO[i].TEXT))->TEXT);
            return 1;
        }
    }
    log_info("Instruction set initialized");
    return 0;
}

void destroy_instructions_set(void)
{
    destroy_dictionary(instructions_by_text);
    log_info("Instruction set destroyed");
}

tll_op_t get_op_code(const char* token)
{
    tll_op_t* op_info = (tll_op_t*) get_item(instructions_by_text, token);

    if (op_info == NULL)
    {
        return (tll_op_t) {OP_ERROR, NO_OPERAND, token};
    }
    return *op_info;
}

