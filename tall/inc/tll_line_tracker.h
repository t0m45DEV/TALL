
/**
 * A struct to hold the info of the line being read
 */
typedef struct {
    char* string_buffer;      // The entire text, this will be shifted to start at the current line
    char* curr_line_content;  // The current line being read, it ends with a '\0' char
    int curr_line_idx;        // The index (starting from 1) of the current line
} tll_line_tracker_t;

/**
 * Creates the initial state for the struct holding the current line info
 */
tll_line_tracker_t init_line_tracker(char* string);

/**
 * Frees the memory used by the given line tracker
 */
void destroy_line_tracker(tll_line_tracker_t* line_tracker);

/**
 * Gets the next line of the given string, and updates line_count to the line index corresponding to the result line
 */
char* next_line(tll_line_tracker_t* line_tracker);

