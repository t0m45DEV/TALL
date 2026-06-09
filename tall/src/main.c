#include "tll_vm.h"

#include "tll_common.h"
#include "tll_file_manager.h"

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
        free_VM();
        fprintf(stderr, "Not enough memory to read \"%s\".\n", file_path);
        exit(74);
    }

    if (!read_file(file_path, source_code))
    {
        free_VM();
        exit(74);
    }
    tll_interpret_result result = interpret_code(source_code);
    free(source_code);

    if (result == TLL_INTERPRET_COMPILE_ERROR)
    {
        exit(65);
    }
    if (result == TLL_INTERPRET_RUNTIME_ERROR)
    {
        exit(70);
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
            exit(EXIT_FAILURE);
        }
        run_file(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: %s [path]\n", argv[0]);
        exit(64);
    }
    free_VM();
    exit(EXIT_SUCCESS);
}

