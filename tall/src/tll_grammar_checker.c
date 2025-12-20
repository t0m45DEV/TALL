#include "tll_grammar_checker.h"

#include "tll_instructions.h"
#include "tll_line_tracker.h"
#include "tll_log.h"
#include "tll_string.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Given a line of code, returns the equivalent instruction parsed a new string (without comments, spaces, etc)
 */
char* parse_line(char* line);

/**
 * Converts the given instruction in format "INT OP", where INT is the operation and OP is the operand, into the struct tll_op_t
 */
const tll_op_t decode_line(const char* line);

int check_grammar(char* code)
{
    int error_count = 0;
    int instruction_set_status = init_instructions_set();

    if (instruction_set_status != 0)
    {
        log_error("There was an error setting up the instruction set!\n");
        destroy_instructions_set();
        return -1;
    }
    tll_line_tracker_t line_tracker = init_line_tracker(code);

    while (advance_line_tracker(&line_tracker) != NULL)
    {
        const char* op_parsed = parse_line(line_tracker.curr_line_content);

        if (op_parsed == NULL)
        {
            log_error("There was an error parsing the line %i\n", line_tracker.curr_line_idx);
            return 1;
        }
        const tll_op_t op_token = decode_line(op_parsed);

        if (op_token.OP_CODE == OP_ERROR)
        {
            error_count++;
        }
        printf("%2i | ", line_tracker.curr_line_idx);
        print_op(op_token);
    }
    destroy_line_tracker(&line_tracker);
    destroy_instructions_set();

    return error_count;
}

char* parse_line(char* line)
{
    remove_comments(line);
    remove_redundant_spaces(line);
    string_to_upper(line);

    return line;
}

const tll_op_t decode_line(const char* line)
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
        log_error("Invalid syntax!\n");
        return (tll_op_t) {OP_ERROR, NO_OPERAND, {}, line};
    }

    for (int i = 0; i < op.OPERANDS_NEEDED; i++)
    {
        if (next_word(&pointer, word, NULL) == NULL)
        {
            free(word);
            log_error("To few arguments for an operation!\n");
            return (tll_op_t) {OP_ERROR, ONE_OPERAND, {}, line};
        }
        char* operand = malloc(strlen(word) * sizeof(char));
        strncpy(operand, word, strlen(word));

        op.OPERANDS[i] = operand;
    }

    if (next_word(&pointer, word, NULL) != NULL)
    {
        free(word);
        log_error("Too many arguments for an operation!\n");
        return (tll_op_t) {OP_ERROR, ONE_OPERAND, {}, line};
    }

    free(word);
    return op;
}

