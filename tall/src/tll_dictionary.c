#include "tll_dictionary.h"

#include "tll_memory.h"
#include "tll_value.h"
#include "tll_object.h"

#include <string.h>
#include <stdint.h>

#define DICTIONARY_MAX_LOAD 0.75

#define DICT_NULL_ENTRY ((tll_dict_entry) {.key = NULL, .value = AS_TLL_NULL})
#define DICT_TOMBSTONE  ((tll_dict_entry) {.key = NULL, .value = AS_TLL_BOOL(true)})

/**
 * Returns the entry inside the entries array with the same key as the given one.
 *
 * Returns NULL if not found.
 */
static tll_dict_entry* find_entry(tll_dict_entry* entries, int capacity, tll_string* key);

/**
 * Adjusts the given dictionary to hold a given capacity of elements.
 */
static void adjust_capacity(tll_dictionary* dictionary, int capacity);

/**
 * Returns true if the given entry is an empty entry.
 */
static inline bool is_empty_entry(const tll_dict_entry* entry);

/**
 * Transform the given entry into an empty entry.
 */
static inline void to_empty_entry(tll_dict_entry* entry);

/**
 * Returns true if the given dictionary entry is a tombstone entry.
 */
static inline bool is_tombstone_entry(const tll_dict_entry* entry);

/**
 * Transform the given entry into an tombstone entry.
 */
static inline void to_tombstone_entry(tll_dict_entry* entry);

void init_dictionary(tll_dictionary* dictionary)
{
    dictionary->count = 0;
    dictionary->capacity = 0;
    dictionary->entries = NULL;
}

void free_dictionary(tll_dictionary* dictionary)
{
    FREE_ARRAY(tll_dict_entry, dictionary->entries, dictionary->capacity);
    dictionary->entries = NULL;
}

bool set_to_dictionary(tll_dictionary* dictionary, tll_string* key, tll_value value)
{
    if (dictionary->count + 1 > dictionary->capacity * DICTIONARY_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(dictionary->capacity);
        adjust_capacity(dictionary, capacity);
    }
    tll_dict_entry* entry = find_entry(dictionary->entries, dictionary->capacity, key);
    bool is_new_key = entry->key == NULL;

    if (is_empty_entry(entry))
    {
        dictionary->count++;
    }
    entry->key = key;
    entry->value = value;

    return is_new_key;
}

bool get_from_dictionary(tll_dictionary* dictionary, tll_string* key, tll_value* value)
{
    if (dictionary->count == 0)
    {
        return false;
    }
    tll_dict_entry* entry = find_entry(dictionary->entries, dictionary->capacity, key);

    if (entry->key == NULL)
    {
        return false;
    }
    *value = entry->value;
    return true;
}

bool delete_from_dictionary(tll_dictionary* dictionary, tll_string* key)
{
    if (dictionary->count == 0)
    {
        return false;
    }

    tll_dict_entry* entry = find_entry(dictionary->entries, dictionary->capacity, key);

    if (entry->key == NULL)
    {
        return false;
    }

    // Place tombstone in the entry.
    to_tombstone_entry(entry);
    return true;
}

void dictionary_add_all(tll_dictionary* source, tll_dictionary* destination)
{
    for (int i = 0; i < source->capacity; i++)
    {
        tll_dict_entry* entry = &source->entries[i];

        if (entry->key != NULL)
        {
            set_to_dictionary(destination, entry->key, entry->value);
        }
    }
}

tll_string* dictionary_find_string(tll_dictionary* dictionary, const char* chars, int length)
{
    if (dictionary->count == 0)
    {
        return NULL;
    }
    uint32_t chars_hash = get_hash(chars, length);
    uint32_t index = chars_hash % dictionary->capacity;
    tll_string* found = NULL;

    while (true)
    {
        tll_dict_entry* entry = &dictionary->entries[index];

        if (is_empty_entry(entry))
        {
            return NULL;
        }

        if (!is_tombstone_entry(entry))
        {
            uint32_t entry_hash = entry->key->hash;

            if (entry->key->length == length && entry_hash == chars_hash && memcmp(entry->key->chars, chars, length) == 0)
            {
                found = entry->key;
                break;
            }
        }
        index = (index + 1) % dictionary->capacity;
    }
    return found;
}

static tll_dict_entry* find_entry(tll_dict_entry* entries, int capacity, tll_string* key)
{
    uint32_t index = key->hash % capacity;
    tll_dict_entry* entry = NULL;
    tll_dict_entry* tombstone = NULL;

    while (true)
    {
        entry = &entries[index];

        if (is_empty_entry(entry))
        {
            // We reach an empty entry but found before a tombstone.
            if (tombstone != NULL)
            {
                entry = tombstone;
            }
            break;
        }
        else if (is_tombstone_entry(entry))
        {
            if (tombstone == NULL)
            {
                tombstone = entry;
            }
        }
        else if (entry->key == key || (entry->key != NULL && entry->key->length == key->length && memcmp(entry->key->chars, key->chars, key->length) == 0))
        {
            // We found the entry.
            break;
        }
        index = (index + 1) % capacity;
    }
    return entry;
}

static void adjust_capacity(tll_dictionary* dictionary, int capacity)
{
    tll_dict_entry* entries = ALLOCATE_ARRAY(tll_dict_entry, capacity);

    for (int i = 0; i < capacity; i++)
    {
        to_empty_entry(&entries[i]);
    }

    dictionary->count = 0;
    for (int i = 0; i < dictionary->capacity; i++)
    {
        tll_dict_entry* entry = &dictionary->entries[i];

        if (entry->key == NULL)
        {
            continue;
        }
        tll_dict_entry* dest = find_entry(entries, capacity, entry->key);

        dest->key = entry->key;
        dest->value = entry->value;
        dictionary->count++;
    }
    FREE_ARRAY(tll_dict_entry, dictionary->entries, dictionary->capacity);
    dictionary->capacity = capacity;
    dictionary->entries = entries;
}

static inline bool is_empty_entry(const tll_dict_entry* entry)
{
    return (entry->key == DICT_NULL_ENTRY.key && are_equals(entry->value, DICT_NULL_ENTRY.value));
}

static inline void to_empty_entry(tll_dict_entry* entry)
{
    entry->key = DICT_NULL_ENTRY.key;
    entry->value = DICT_NULL_ENTRY.value;
}

static inline bool is_tombstone_entry(const tll_dict_entry* entry)
{
    return (entry->key == DICT_TOMBSTONE.key && are_equals(entry->value, DICT_TOMBSTONE.value));
}

static inline void to_tombstone_entry(tll_dict_entry* entry)
{
    entry->key = DICT_TOMBSTONE.key;
    entry->value = DICT_TOMBSTONE.value;
}

