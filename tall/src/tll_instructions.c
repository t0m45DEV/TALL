#include "tll_instructions.h"

#include "tll_dictionary.h"
#include "tll_log.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

tll_dictionary_t* instructions_by_text;

const tll_op_t OP_INFO[] = {
    {
        OP_PASS,
        NO_OPERAND,
        {},
        "PASS"
    },
    {
        OP_PUSH,
        ONE_OPERAND,
        {},
        "PUSH"
    },
    {
        OP_POP,
        NO_OPERAND,
        {},
        "POP"
    },
        {
        OP_PEEK,
        ONE_OPERAND,
        {},
        "PEEK"
    },
    {
        OP_CLEAR,
        NO_OPERAND,
        {},
        "CLEAR"
    },
    {
        OP_SHOWF,
        NO_OPERAND,
        {},
        "SHOWF"
    },
    {
        OP_SHOWI,
        NO_OPERAND,
        {},
        "SHOWI"
    },
    {
        OP_SHOWC,
        NO_OPERAND,
        {},
        "SHOWC"
    },
    {
        OP_ADD,
        NO_OPERAND,
        {},
        "ADD"
    },
    {
        OP_SUB,
        NO_OPERAND,
        {},
        "SUB"
    },
    {
        OP_MUL,
        NO_OPERAND,
        {},
        "MUL"
    },
    {
        OP_DIV,
        NO_OPERAND,
        {},
        "DIV"
    },
    {
        OP_MOD,
        NO_OPERAND,
        {},
        "MOD"
    },
};

int init_instructions_set(void)
{
    log_info("Setting up instruction set...\n");
    instructions_by_text = create_dictionary();

    if (!instructions_by_text)
    {
        log_error("Dictionary for instructions set failed!\n");
        return 1;
    }

    for (tll_op_code_t i = 0; i < OP_CODES_COUNT; i++)
    {
        int status = add_item(instructions_by_text, OP_INFO[i].TEXT, (void*) &OP_INFO[i]);

        if (status != 0)
        {
            log_error("There is hashing collision between %s and %s\n",
                      OP_INFO[i].TEXT,
                      ((tll_op_t*) get_item(instructions_by_text, OP_INFO[i].TEXT))->TEXT);
            return 1;
        }
    }
    log_info("Instruction set initialized\n");
    return 0;
}

void destroy_instructions_set(void)
{
    destroy_dictionary(instructions_by_text);
    log_info("Instruction set destroyed\n");
}

tll_op_t get_op_code(const char* token)
{
    tll_op_t* op_info = (tll_op_t*) get_item(instructions_by_text, token);

    if (op_info == NULL)
    {
        return (tll_op_t) {OP_ERROR, NO_OPERAND, {}, token};
    }
    return *op_info;
}

void print_op(const tll_op_t op)
{
    printf("OP_code: 0x%02x | ", op.OP_CODE);
    printf("INT: %4s | ", op.TEXT);

    printf("First: ");

    if (op.OPERANDS[0] != NULL)
    {
        printf("%4i", atoi(op.OPERANDS[0]));
    }
    else
    {
        printf("null");
    }
    printf(" | Second: ");

    if (op.OPERANDS[1] != NULL)
    {
        printf("%4i", atoi(op.OPERANDS[1]));
    }
    else
    {
        printf("null");
    }
    printf("\n");
}

const tll_op_t* copy_op(const tll_op_t op)
{
    tll_op_t* copy = (tll_op_t*) malloc(sizeof(tll_op_t));

    memcpy(copy, &op, sizeof(tll_op_t));

    return copy;
}

