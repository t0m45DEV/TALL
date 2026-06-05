#include "tll_vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static char* read_file(const char* file_path)
{
    FILE* file = fopen(file_path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", file_path);
        exit(74);
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);

    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", file_path);
        exit(74);
    }
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

    if (bytes_read < file_size)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", file_path);
        exit(74);
    }
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(const char* file_path)
{
    char* source_code = read_file(file_path);
    tll_interpret_result result = interpret_code(source_code);
    free(source_code);

    if (result == TLL_INTERPRET_COMPILE_ERROR) exit(65);
    if (result == TLL_INTERPRET_RUNTIME_ERROR) exit(70);
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

