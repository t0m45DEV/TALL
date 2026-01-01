#include "tll_execution.h"

#include "tll_instructions.h"
#include "tll_stack.h"
#include "tll_log.h"

#include <stdio.h>
#include <stdlib.h>

int execute(const tll_op_t* bytecode[], int size)
{
    log_info("Executing the bytecode...\n");

    int status = 0;

    // The pointer to the instruction to be executed
    int ip;

    for (ip = 0; ip < size; ip++)
    {
        const tll_op_t curr_int = *bytecode[ip];

        float a;
        float b;

        switch (curr_int.OP_CODE)
        {
            case OP_ERROR:
                log_error("Tried to execute an invalid operation!\n");
                return 1;
            case OP_CODES_COUNT:
                log_error("Tried to execute an invalid operation!\n");
                return 1;

            case OP_PASS:
                break;

            case OP_PUSH:
                stack_push(atof(curr_int.OPERANDS[0]));
                break;
            case OP_POP:
                stack_pop(&a);
                break;
            case OP_PEEK:
                stack_peek(&a, atoi(curr_int.OPERANDS[0]));
                stack_push(a);
                break;
            case OP_CLEAR:
                stack_clear();
                break;

            case OP_SHOWF:
                stack_head(&a);
                printf("%f", a);
                break;
            case OP_SHOWI:
                stack_head(&a);
                printf("%i", (int) a);
                break;
            case OP_SHOWC:
                stack_head(&a);
                printf("%c", (char) a);
                break;

            case OP_ADD:
                stack_pop(&a);
                stack_pop(&b);
                stack_push(a + b);
                break;
            case OP_SUB:
                stack_pop(&a);
                stack_pop(&b);
                stack_push(b - a);
                break;
            case OP_MUL:
                stack_pop(&a);
                stack_pop(&b);
                stack_push(a * b);
                break;
            case OP_DIV:
                stack_pop(&a);
                stack_pop(&b);
                stack_push(a / b);
                break;
            case OP_MOD:
                stack_pop(&a);
                stack_pop(&b);
                stack_push((int) a % (int) b);
                break;

            case OP_JMP:
                // The operand will come in the form of a line number, not an array index, so we offset it by 1
                ip = atoi(curr_int.OPERANDS[0]) - 1;
                ip--; // To skip the increment part of the for loop
                break;
        }
    }
    log_info("Bytecode executed succesfully\n");
    return status;
}

