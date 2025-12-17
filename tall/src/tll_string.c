#include "tll_string.h"

#include <string.h>
#include <stdbool.h>

#define STR_END '\0'
#define STR_SPACE ' '

#define COMMENT_START ';'

void remove_comments(char* bytecode_line)
{
    int i = 0;

    while (bytecode_line[i] != STR_END)
    {
        if (bytecode_line[i] == COMMENT_START)
        {
            bytecode_line[i] = STR_END;
        }
        i++;
    }
}

void remove_redundant_spaces(char* string)
{
    // Removes the starting spaces
    int i = 0;
    int j = 0;

    while (string[i] == STR_SPACE)
    {
        i++;
    }

    while (string[i] != STR_END)
    {
        string[j] = string[i];
        i++;
        j++;
    }
    string[j] = STR_END;

    // Removes the consecutives spaces
    int length = strlen(string);

    j = 0;
    bool last_char_not_space = true;

    for (i = 0; i < length; i++)
    {
        if (string[i] != STR_SPACE || last_char_not_space)
        {
            string[j] = string[i];
            j++;
        }
        last_char_not_space = (string[i] != STR_SPACE);
    }
    string[j] = STR_END;
}

