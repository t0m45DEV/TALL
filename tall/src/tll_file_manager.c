#include "tll_file_manager.h"

#include "tll_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITE_MODE "wb"
#define READ_MODE  "rb"

const char* get_file_extension(const char* file_path)
{
    const char* file_dot = strchr(file_path, '.');

    if (!file_dot)
    {
        return NULL;
    }
    return file_dot + 1;
}

bool is_tll_file(const char* file_path)
{
    const char* extension = get_file_extension(file_path);

    if (!extension)
    {
        return false;
    }
    return (strcmp(extension, "tll") == 0);
}

const char* get_file_content(const char* file_path)
{
    FILE* file = fopen(file_path, READ_MODE);

    if (file == NULL)
    {
        log_error("There was a error opening %s\n", file_path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);

    int file_len = ftell(file);
    rewind(file);

    char* content = malloc(file_len + 1);

    if (content == NULL)
    {
        log_error("There was an error allocating the buffer for %s\n", file_path);
        return NULL;
    }

    size_t read_size = fread(content, 1, file_len, file);

    if (read_size == 0)
    {
        log_error("There was an error reading %s\n", file_path);
        free(content);
        return NULL;
    }
    content[file_len] = '\0';

    fclose(file);

    return (const char*) content;
}

void delete_file_content(const char* file_content)
{
    free((void*) file_content);
}

