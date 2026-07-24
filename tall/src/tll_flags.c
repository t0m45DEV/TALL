#include "tll_flags.h"

#include <stdio.h>
#include <unistd.h>

// True if the user used the debug printing flag for the parser step.
bool flag_parser = false;

// True if the user used the debug printing flag for the bytecode generation step.
bool flag_bytecode = false;

// True if the user used the trace code execution flag.
bool flag_trace = false;

// True if the user used the help flag.
bool flag_help = false;

// True if the user used the version flag.
bool flag_version = false;

bool check_flags(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "pbdthv")) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                flag_parser = true;
                break;
            }
            case 'b':
            {
                flag_bytecode = true;
                break;
            }
            case 'd':
            {
                flag_parser = true;
                flag_bytecode = true;
                break;
            }
            case 't':
            {
                flag_trace = true;
                break;
            }
            case 'h':
            {
                flag_help = true;
                break;
            }
            case 'v':
            {
                flag_version = true;
                break;
            }
            case '?': // Unkown flag.
            {
                return false;
            }
        }
    }
    return true;
}

bool is_flag_argument(const char* arg)
{
    return (arg[0] == '-');
}

bool is_debug_parser_flag(void)
{
    return flag_parser;
}

bool is_debug_bytecode_flag(void)
{
    return flag_bytecode;
}

bool is_trace_flag(void)
{
    return flag_trace;
}

bool is_help_flag(void)
{
    return flag_help;
}

bool is_version_flag(void)
{
    return flag_version;
}

