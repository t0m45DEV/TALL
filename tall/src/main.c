#include "tll_log.h"
#include "tll_file_manager.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
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

    log_info("The file name is %s", argv[1]);
    log_info("File content:\n%s", file_content);
    delete_file_content(file_content);
    exit(EXIT_SUCCESS);
}

