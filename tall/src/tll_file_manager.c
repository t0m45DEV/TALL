#include "tll_file_manager.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char* get_file_extension(const char* file_path)
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

size_t file_size(const char* file_path)
{
    FILE* file = fopen(file_path, "rb");

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fclose(file);

    return file_size;
}

int read_file(const char* file_path, char* buffer)
{
    FILE* file = fopen(file_path, "rb");

    if (file == NULL)
    {
        fclose(file);
        fprintf(stderr, "Could not open file \"%s\".\n", file_path);
        return 1;
    }
    size_t f_size = file_size(file_path);

    size_t bytes_read = fread(buffer, sizeof(char), f_size, file);

    if (bytes_read < f_size)
    {
        fclose(file);
        fprintf(stderr, "Could not read file \"%s\".\n", file_path);
        return 1;
    }
    buffer[bytes_read] = '\0';

    fclose(file);
    return 1;
}

