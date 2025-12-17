#include "tll_control_unit.h"

#include "tll_string.h"

#include <stdio.h>

int process_code(char* code)
{
    tll_line_tracker_t line_tracker = init_line_tracker(code);

    while (next_line(&line_tracker) != NULL)
    {
        const char* operation = fetch_op(line_tracker.curr_line_content);

        if (operation == NULL)
        {
            printf("There was an error parsing the line %i\n", line_tracker.curr_line_idx);
            return 1;
        }
        if (*operation == '\0')
        {
            continue;
        }
        printf("%02i : %s\n", line_tracker.curr_line_idx, operation);
    }
    destroy_line_tracker(&line_tracker);

    return 0;
}

char* fetch_op(char* bytecode_line)
{
    remove_comments(bytecode_line);
    remove_redundant_spaces(bytecode_line);

    return bytecode_line;
}

const tll_op_t decode_op(char* token)
{
    return (tll_op_t) {0};
}

int execute_op(const tll_op_t op)
{
    return 0;
}

