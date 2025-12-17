#include "tll_control_unit.h"

#include "tll_instructions.h"
#include "tll_line_tracker.h"
#include "tll_string.h"

#include <stdio.h>

int process_code(char* code)
{
    tll_line_tracker_t line_tracker = init_line_tracker(code);

    while (advance_line_tracker(&line_tracker) != NULL)
    {
        const char* op_parsed = fetch_op(line_tracker.curr_line_content);

        if (op_parsed == NULL)
        {
            printf("There was an error parsing the line %i\n", line_tracker.curr_line_idx);
            return 1;
        }
        printf("%02i: %s\n", line_tracker.curr_line_idx, op_parsed);

        const tll_op_t op_token = decode_op(op_parsed);

        printf("OP Code: %0i\n", op_token.OP_CODE);
    }
    destroy_line_tracker(&line_tracker);

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
        return (tll_op_t) {OP_PASS, 0};
    }
    return (tll_op_t) {get_op_code(line), 0};
}

int execute_op(const tll_op_t op)
{
    return 0;
}

