#include "tll_control_unit.h"

#include "tll_string.h"

#include <stdio.h>
#include <string.h>

#define NEW_LINE "\n"

int process_code(char* code)
{
    char* line = strtok(code, NEW_LINE);
    int line_cnt = 0;

    while (line != NULL)
    {
        const char* operation = fetch_op(line);

        if (operation == NULL)
        {
            printf("There was an error parsing the line %i\n", line_cnt);
            return 1;
        }
        if (*operation == '\0')
        {
            line = strtok(NULL, NEW_LINE);
            line_cnt++;
            continue;
        }
        printf("%i : %s\n", line_cnt, operation);

        line = strtok(NULL, NEW_LINE);
        line_cnt++;
    }
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


