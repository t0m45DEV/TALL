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
 * Returns the ammount of lines in the given string
 */
int get_line_count(const char* string);

/**
 * Gets the next token of the given string delimited by the given delimiter, and updates found_cnt to the token index corresponding to the result token
 *
 * Returns NULL if not token was found
 */
char* next_token(char** string, char delimiter, char* token, int* found_cnt);

/**
 * Same as next_token, but with entire lines
 */
char* next_line(char** string, char* token, int* found_cnt);

/**
 * Same as next_token, but with words
 */
char* next_word(char** string, char* token, int* found_cnt);


#endif
