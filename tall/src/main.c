#include "tll_vm.h"
#include "tll_flags.h"
#include "tll_common.h"
#include "tll_file_manager.h"
#include "tll_memory.h"

#include <stddef.h>
#include <sysexits.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Boots up the interactive interpreter for TALL.
 */
static void repl(void);

/**
 * Compiles to TALL bytecode and runs through the TALL VM the contents of the given file.
 */
static void run_file(const char* file_path);

int main(int argc, char* argv[])
{
    if (!check_flags(argc, argv))
    {
        fprintf(stderr, "Unkown flag, try running 'tall -h' for the help page.\n");
        exit(EX_USAGE);
    }

    if (is_version_flag())
    {
        printf("TALL v0.12 by t0m45DEV.\n");
        exit(EX_OK);
    }
    else if (is_help_flag())
    {
        printf("Usage: %s [-v] [-h] [-t] [-p] [-b] [-d] [path_to_file]\n", strip_path(argv[0]));
        printf("  -v  print version\n");
        printf("  -h  print this help\n");
        printf("  -t  debug print the state of the stack on each instruction\n");
        printf("  -p  debug print parser step\n");
        printf("  -b  debug print bytecode step\n");
        printf("  -d  debug print both parser and bytecode steps\n");
        exit(EX_OK);
    }
    // After this point, we run the compiler as usual.
    init_VM();

    if (argc == 1)
    {
        repl();
    }
    else
    {
        char* file_path = NULL;

        for (int i = 1; i < argc; i++)
        {
            if (!is_flag_argument(argv[i]))
            {
                if (file_path != NULL)
                {
                    free_VM();
                    fprintf(stderr, "Usage: %s [-flags] [path]\n", strip_path(argv[0]));
                    exit(EX_USAGE);
                }
                file_path = argv[i];
            }
        }

        if (!is_tll_file(file_path))
        {
            free_VM();
            fprintf(stderr, "File %s has not '.tll' extension.\n", file_path);
            exit(EX_DATAERR);
        }
        run_file(file_path);
    }
    free_VM();
    exit(EX_OK);
}

static void repl(void)
{
    char line[1024];

    while (true)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }
        interpret_code(line);
    }
}

static void run_file(const char* file_path)
{
    size_t f_size = file_size(file_path) + 1; // Last one byte for \0 terminator.

    if (f_size - 1 == (size_t) -1)
    {
        free_VM();
        exit(EX_IOERR);
    }
    else if (f_size == (size_t) 1) // Blank file, nothing to compile.
    {
        free_VM();
        exit(EX_OK);
    }
    char* source_code = ALLOCATE_ARRAY(char, f_size);

    if (source_code == NULL)
    {
        FREE_ARRAY(char, source_code, f_size);
        free_VM();
        fprintf(stderr, "Not enough memory to read \"%s\".\n", file_path);
        exit(EX_OSERR);
    }

    if (!read_file(file_path, source_code))
    {
        FREE_ARRAY(char, source_code, f_size);
        free_VM();
        exit(EX_IOERR);
    }
    tll_interpret_result result = interpret_code(source_code);
    FREE_ARRAY(char, source_code, f_size);

    if (result == TLL_INTERPRET_COMPILE_ERROR)
    {
        free_VM();
        exit(EX_DATAERR);
    }
    if (result == TLL_INTERPRET_RUNTIME_ERROR)
    {
        free_VM();
        exit(EX_SOFTWARE);
    }
}

