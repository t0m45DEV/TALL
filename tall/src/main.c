#include "tll_log.h"
#include "tll_file_manager.h"
#include "tll_control_unit.h"

#include <stdlib.h>

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
    int status = process_code((char*) file_content);

    if (status != 0)
    {
        log_error("There was an error parsing the file!\n");
        exit(EXIT_FAILURE);
    }
    delete_file_content(file_content);

    log_info("Neat code\n");
    exit(EXIT_SUCCESS);
}

