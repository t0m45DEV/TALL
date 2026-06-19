#ifndef _TLL_DICTIONARY_H
#define _TLL_DICTIONARY_H

#include "tll_common.h"
#include "tll_value.h"

#include <stdbool.h>

/**
 * An entry for a dictionary, basically a key and a value.
 */
typedef struct {
    tll_string* key;
    tll_value value;
} tll_dict_entry;

/**
 * A collection of values that can be accessed with a key.
 */
typedef struct {
    int count;
    int capacity;
    tll_dict_entry* entries;
} tll_dictionary;

/**
 * Initialize the given dictionary.
 */
void init_dictionary(tll_dictionary* dictionary);

/**
 * Frees the memory being used by the given dictionary.
 */
void free_dictionary(tll_dictionary* dictionary);

/**
 * Adds the given value to the given dictionary using the given key as index if the key is not being used. Changes the value at that position if it already exists.
 *
 * Returns true if the given key is not already in use.
 */
bool set_to_dictionary(tll_dictionary* dictionary, tll_string* key, tll_value value);

/**
 * Returns true if the given key is already used in the given dictionary. If that's the case, the given value pointer is the value found with the given key.
 */
bool get_from_dictionary(tll_dictionary* dictionary, tll_string* key, tll_value* value);

/**
 * Deletes the value from the given dictionary at the given key position.
 *
 * Returns true if the given key was already being used.
 */
bool delete_from_dictionary(tll_dictionary* dictionary, tll_string* key);

/**
 * Copy all the entries from the given source dictionary to the given destination dictionary.
 */
void dictionary_add_all(tll_dictionary* source, tll_dictionary* destination);

/**
 * Returns the TLL string in the dictionary that has the same contents as the given chars and length. If is not on the dictionary, returns NULL.
 */
tll_string* dictionary_find_string(tll_dictionary* dictionary, const char* chars, int length);

#endif
