#include "tll_instructions.h"
#include "tll_log.h"
#include "tll_file_manager.h"
#include "tll_grammar_checker.h"
#include "tll_string.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    log_info("TALL Interpreter v0.2\n");

    if (argc < 2)
    {
        log_error("File name not given!\n");
        exit(EXIT_FAILURE);
    }

    if (!is_tll_file(argv[1]))
    {
        log_error("The file given is not a .tll file!\n");
        exit(EXIT_FAILURE);
    }
    const char* file_content = get_file_content(argv[1]);

    if (file_content == NULL)
    {
        log_error("Getting the file content failed!\n");
        exit(EXIT_FAILURE);
    }
    const tll_op_t* bytecode[get_line_count(file_content)];

    int error_count = check_grammar((char*) file_content, bytecode);

    delete_file_content(file_content);

    if (error_count != 0)
    {
        if (error_count == -1)
        {
            log_error("There was an error in the grammar checking!\n");
        }
        else if (error_count == 1)
        {
            log_error("The file has %i error!\n", error_count);
        }
        else
        {
            log_error("The file has %i errors!\n", error_count);
        }
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < get_line_count(file_content); i++)
    {
        free((void*) bytecode[i]);
    }
    log_info("Neat code\n");
    exit(EXIT_SUCCESS);
}

