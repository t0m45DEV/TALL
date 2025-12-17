#include "tll_string.h"

#include <stdlib.h>
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

tll_line_tracker_t init_line_tracker(char* string)
{
    tll_line_tracker_t line_tracker = {0};

    line_tracker.string_buffer = string;
    line_tracker.curr_line_content = malloc(strlen(string) * sizeof(char));
    line_tracker.curr_line_idx = 0;

    return line_tracker;
}

void destroy_line_tracker(tll_line_tracker_t* line_tracker)
{
    free(line_tracker->curr_line_content);
}

char* next_line(tll_line_tracker_t* line_tracker)
{
    if (!line_tracker->string_buffer || *line_tracker->string_buffer == CHAR_END)
    {
        return NULL;
    }

    char* line_start = line_tracker->string_buffer;
    char* line_end = line_start;

    while (*line_end != CHAR_NEW_LINE && *line_end != CHAR_END)
    {
        line_end++;
    }

    size_t line_length = line_end - line_start;
    strncpy(line_tracker->curr_line_content, line_start, line_length);
    line_tracker->curr_line_content[line_length] = CHAR_END;

    if (*line_end == CHAR_NEW_LINE)
    {
        line_tracker->string_buffer = line_end + 1;
        line_tracker->curr_line_idx += 1;
    }
    else
    {
        // We get here if line_end points to a CHAR_END (end of string)
        line_tracker->string_buffer = NULL;
    }
    return line_tracker->curr_line_content;
}

