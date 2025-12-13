
/**
 * Returns the content of the given file in the form of a string
 *
 * @note Caller MUST free the returned pointer
 */
const char* get_file_content(const char* filepath);

/**
 * Delete the buffer for the given file_content
 */
void delete_file_content(const char* file_content);

