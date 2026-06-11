#include "tll_vm.h"

#include "tll_common.h"
#include "tll_file_manager.h"

#include <sysexits.h>
#include <stdio.h>
#include <stdlib.h>

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
    char* source_code = (char*) malloc(file_size(file_path) + 1);

    if (source_code == NULL)
    {
        free(source_code);
        free_VM();
        fprintf(stderr, "Not enough memory to read \"%s\".\n", file_path);
        exit(EX_OSERR);
    }

    if (!read_file(file_path, source_code))
    {
        free(source_code);
        free_VM();
        exit(EX_IOERR);
    }
    tll_interpret_result result = interpret_code(source_code);
    free(source_code);

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

int main(int argc, const char* argv[])
{
    init_VM();

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        if (!is_tll_file(argv[1]))
        {
            free_VM();
            fprintf(stderr, "File %s has not \".tll\" extension.\n", argv[1]);
            exit(EX_DATAERR);
        }
        run_file(argv[1]);
    }
    else
    {
        free_VM();
        fprintf(stderr, "Usage: %s [path]\n", argv[0]);
        exit(EX_USAGE);
    }
    free_VM();
    exit(EX_OK);
}

