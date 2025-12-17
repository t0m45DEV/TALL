#include "tll_dictionary.h"

#include <stdlib.h>

/**
 * Returns the corresponding hash to the given string, using the SDBM hashing algorithm
 */
int get_hash(char* string)
{
    int hash = 0;

    for (int i = 0; string[i] != '\0'; i++)
    {
        hash = string[i] + (hash << 6) + (hash << 16) - hash;
    }
    return (hash % DICT_MAX_ELEMENTS);
}

tll_dictionary_t* create_dictionary(void)
{
    tll_dictionary_t* new_dict = (tll_dictionary_t*) malloc(sizeof(tll_dictionary_t));

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

void add_item(tll_dictionary_t* dict, char* key, void* item)
{
    if (dict->elements[get_hash(key)] == NULL)
    {
        dict->elements[get_hash(key)] = item;
        dict->size += 1;
    }
}

void* get_item(const tll_dictionary_t* dict, char* key)
{
    return dict->elements[get_hash(key)];
}

void replace_item(tll_dictionary_t* dict, char* key, void* new_item)
{
    if (dict->elements[get_hash(key)] != NULL)
    {
        dict->elements[get_hash(key)] = new_item;
    }
}

void delete_item(tll_dictionary_t* dict, char* key)
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

