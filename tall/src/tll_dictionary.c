#include "tll_dictionary.h"
#include "tll_log.h"

#include <stdlib.h>

/**
 * Returns the corresponding hash to the given string, using the SDBM hashing algorithm
 */
int get_hash(const char* string)
{
    unsigned int hash = 0;

    for (int i = 0; string[i] != '\0'; i++)
    {
        hash = string[i] + (hash << 6) + (hash << 16) - hash;
    }
    hash = hash % DICT_MAX_ELEMENTS; // Leave it at a valid range

    return hash;
}

tll_dictionary_t* create_dictionary(void)
{
    tll_dictionary_t* new_dict = (tll_dictionary_t*) malloc(sizeof(tll_dictionary_t));

    if (new_dict == NULL)
    {
        log_error("The space for the dictionary could not be allocated\n");
        return NULL;
    }

    for (int i = 0; i < DICT_MAX_ELEMENTS; i++)
    {
        new_dict->elements[i] = NULL;
    }
    new_dict->size = 0;

    return new_dict;
}

int get_size(const tll_dictionary_t *dict)
{
    return dict->size;
}

int add_item(tll_dictionary_t* dict, const char* key, void* item)
{
    int status = 1;

    if (dict->elements[get_hash(key)] == NULL)
    {
        dict->elements[get_hash(key)] = item;
        dict->size += 1;
        status = 0;
    }
    return status;
}

void* get_item(const tll_dictionary_t* dict, const char* key)
{
    return dict->elements[get_hash(key)];
}

void replace_item(tll_dictionary_t* dict, const char* key, void* new_item)
{
    if (dict->elements[get_hash(key)] != NULL)
    {
        dict->elements[get_hash(key)] = new_item;
    }
}

void delete_item(tll_dictionary_t* dict, const char* key)
{
    if (dict->elements[get_hash(key)] != NULL)
    {
        dict->elements[get_hash(key)] = NULL;
        dict->size -= 1;
    }
}

void destroy_dictionary(tll_dictionary_t* dict)
{
    free(dict);
}

