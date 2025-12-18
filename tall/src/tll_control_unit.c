#include "tll_control_unit.h"

#include "tll_instructions.h"
#include "tll_line_tracker.h"
#include "tll_log.h"
#include "tll_string.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int process_code(char* code)
{
    int instruction_set_status = init_instructions_set();

    if (instruction_set_status != 0)
    {
        log_error("There was an error setting up the instruction set!");
        destroy_instructions_set();
        return 1;
    }
    tll_line_tracker_t line_tracker = init_line_tracker(code);

    while (advance_line_tracker(&line_tracker) != NULL)
    {
        const char* op_parsed = fetch_op(line_tracker.curr_line_content);

        if (op_parsed == NULL)
        {
            printf("There was an error parsing the line %i\n", line_tracker.curr_line_idx);
            return 1;
        }
        const tll_op_t op_token = decode_op(op_parsed);
        printf("%2i | ", line_tracker.curr_line_idx);
        print_op(op_token);
    }
    destroy_line_tracker(&line_tracker);
    destroy_instructions_set();

    return 0;
}

char* fetch_op(char* bytecode_line)
{
    remove_comments(bytecode_line);
    remove_redundant_spaces(bytecode_line);
    string_to_upper(bytecode_line);

    return bytecode_line;
}

const tll_op_t decode_op(const char* line)
{
    if (*line == '\0')
    {
        return get_op_code("PASS");
    }
    char* word = malloc(strlen(line) * sizeof(char));
    char* pointer = (char*) line;
    next_word(&pointer, word, NULL);

    tll_op_t op = get_op_code(word);

    if (op.OP_CODE == OP_ERROR)
    {
        free(word);
        log_error("Invalid syntax!");
        return (tll_op_t) {OP_ERROR, NO_OPERAND, {}, line};
    }

    for (int i = 0; i < op.OPERANDS_NEEDED; i++)
    {
        if (next_word(&pointer, word, NULL) == NULL)
        {
            free(word);
            log_error("To few arguments for an operation!");
            return (tll_op_t) {OP_ERROR, ONE_OPERAND, {}, line};
        }
        char* operand = malloc(strlen(word) * sizeof(char));
        strncpy(operand, word, strlen(word));

        op.OPERANDS[i] = operand;
    }

    if (next_word(&pointer, word, NULL) != NULL)
    {
        free(word);
        log_error("Too many arguments for an operation!");
        return (tll_op_t) {OP_ERROR, ONE_OPERAND, {}, line};
    }

    free(word);
    return op;
}

int execute_op(const tll_op_t op)
{
    return 0;
}

