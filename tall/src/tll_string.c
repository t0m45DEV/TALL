#include "tll_string.h"

#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define CHAR_END      '\0'
#define CHAR_SPACE    ' '
#define CHAR_NEW_LINE '\n'

#define STR_NEWLINE   "\n"

#define COMMENT_START ';'

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

void string_to_upper(char* string)
{
    char* pointer = string;

    while (*pointer != CHAR_END)
    {
        *pointer = toupper(*pointer);
        pointer++;
    }
}

char* next_token(char** string, char delimiter, char* token, int* found_cnt)
{
    if (!string || !*string || **string == CHAR_END)
    {
        return NULL;
    }

    char* token_start = *string;
    char* token_end = token_start;

    while (*token_end != delimiter && *token_end != CHAR_END)
    {
        token_end++;
    }

    size_t token_length = token_end - token_start;
    strncpy(token, token_start, token_length);
    token[token_length] = CHAR_END;

    if (*token_end == delimiter)
    {
        *string = token_end + 1;

        if (found_cnt) { *found_cnt += 1; }
    }
    else
    {
        // We get here if token_end points to a CHAR_END (end of string)
        string = NULL;
    }

    return token;
}

char* next_line(char** string, char* token, int* found_cnt)
{
    return next_token(string, CHAR_NEW_LINE, token, found_cnt);
}

char* next_word(char** string, char* token, int* found_cnt)
{
    return next_token(string, CHAR_SPACE, token, found_cnt);
}

