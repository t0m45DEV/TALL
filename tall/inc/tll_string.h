
#define CHAR_END      '\0'
#define CHAR_SPACE    ' '
#define CHAR_NEW_LINE '\n'

#define STR_NEWLINE   "\n"

#define COMMENT_START ';'

/**
 * Removes from the given string all the things after a semicolon
 */
void remove_comments(char* bytecode_line);

/**
 * Delete the spaces at the start of the given string, and the spaces that are consecutive
 */
void remove_redundant_spaces(char* string);

/**
 * Turns every char in the string to UPPERCASE
 */
void string_to_upper(char* string);

