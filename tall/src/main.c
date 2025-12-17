#include "tll_log.h"
#include "tll_file_manager.h"
#include "tll_control_unit.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    log_info("TALL Interpreter v0.2");

    if (argc < 2)
    {
        log_error("File name not given!");
        exit(EXIT_FAILURE);
    }

    if (!is_tll_file(argv[1]))
    {
        log_error("The file given is not a .tll file!");
        exit(EXIT_FAILURE);
    }
    const char* file_content = get_file_content(argv[1]);

    if (file_content == NULL)
    {
        log_error("Getting the file content failed!");
        exit(EXIT_FAILURE);
    }
    // The string will be modified, we need to copy it so later we can free it
    char* code = (char*) file_content;

    int status = process_code(code);

    if (status != 0)
    {
        log_error("There was an error parsing the file!");
        exit(EXIT_FAILURE);
    }
    delete_file_content(file_content);

    log_info("Neat code");
    exit(EXIT_SUCCESS);
}

