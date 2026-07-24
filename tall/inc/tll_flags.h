#include <stdbool.h>

/**
 * Checks if the user wrote some flags on the given argument list.
 *
 * Returns true if the flag parsing went alright.
 */
bool check_flags(int argc, char* argv[]);

/**
 * Returns true if the given string is a flag argument.
 */
bool is_flag_argument(const char* arg);

/**
 * Returns true if the user asked for the debug printing for the parsing step.
 */
bool is_debug_parser_flag(void);

/**
 * Returns true if the user asked for the debug printing for the bytecode generation step.
 */
bool is_debug_bytecode_flag(void);

/**
 * Returns true if the user asked for the trace code execution.
 */
bool is_trace_flag(void);

/**
 * Returns true if the user asked for the help page info.
 */
bool is_help_flag(void);

/**
 * Returns true if the user asked for the program version info.
 */
bool is_version_flag(void);

