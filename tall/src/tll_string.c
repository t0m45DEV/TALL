#include "tll_string.h"

#include <string.h>
#include <stdbool.h>

void remove_comments(char* bytecode_line)
{
    int i = 0;

    while (bytecode_line[i] != CHAR_END)
    {
        if (bytecode_line[i] == COMMENT_START)
        {
            bytecode_line[i] = CHAR_END;
        }
        i++;
    }
}

void remove_redundant_spaces(char* string)
{
    // Removes the starting spaces
    int i = 0;
    int j = 0;

    while (string[i] == CHAR_SPACE)
    {
        i++;
    }

    while (string[i] != CHAR_END)
    {
        string[j] = string[i];
        i++;
        j++;
    }
    string[j] = CHAR_END;

    // Removes the consecutives spaces
    int length = strlen(string);

    j = 0;
    bool last_char_not_space = true;

    for (i = 0; i < length; i++)
    {
        if (string[i] != CHAR_SPACE || last_char_not_space)
        {
            string[j] = string[i];
            j++;
        }
        last_char_not_space = (string[i] != CHAR_SPACE);
    }
    string[j] = CHAR_END;
}

