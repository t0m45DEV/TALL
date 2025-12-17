#include "tll_line_tracker.h"

#include "tll_string.h"

#include <stdlib.h>
#include <string.h>

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

char* advance_line_tracker(tll_line_tracker_t* line_tracker)
{
    return next_line(&line_tracker->string_buffer,
                      line_tracker->curr_line_content,
                      &line_tracker->curr_line_idx
    );
}

