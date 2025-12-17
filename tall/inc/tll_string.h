#ifndef _TLL_STRING
#define _TLL_STRING

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

/**
 *
 */
char* next_token(char** string, char delimiter, char* token, int* found_cnt);

/**
 *
 */
char* next_line(char** string, char* token, int* found_cnt);

/**
 *
 */
char* next_word(char** string, char* token, int* found_cnt);


#endif
