#ifndef _TLL_FILE_MANAGER
#define _TLL_FILE_MANAGER

#include <stdbool.h>

/**
 * Returns the extension of the given file
 *
 * @note Returns NULL if the file has no extension
 */
const char* get_file_extension(const char* file_path);

/**
 * Returns true if the given file has a .tll
 *  extension, otherwise returns false
 */
bool is_tll_file(const char* file_path);

/**
 * Returns the content of the given file in the form of a string
 *
 * @note Caller MUST free the returned pointer
 */
const char* get_file_content(const char* file_path);

/**
 * Delete the buffer for the given file_content
 */
void delete_file_content(const char* file_content);

#endif
