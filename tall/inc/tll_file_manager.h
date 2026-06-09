#ifndef _TLL_FILE_MANAGER_H
#define _TLL_FILE_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>

/**
 * Returns true if the given file has a '.tll' extension.
 */
bool is_tll_file(const char* file_path);

/**
 * Returns the size in bytes of the given file.
 */
size_t file_size(const char* file_path);

/**
 * Saves the content of the given file in the given buffer.
 *
 * Returns 0 if the read was OK, returns 1 otherwise.
 */
int read_file(const char* file_path, char* buffer);

#endif
