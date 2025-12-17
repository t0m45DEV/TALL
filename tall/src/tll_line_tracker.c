#include "tll_line_tracker.h"

#include "tll_string.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

