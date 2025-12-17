#ifndef _TLL_DICT
#define _TLL_DICT

#define DICT_MAX_ELEMENTS 101

typedef struct {
    void* elements[DICT_MAX_ELEMENTS];
    int size;
} tll_dictionary_t;

/**
 * Set up the initial state for a generic dictionary
 *
 * @note After use, caller MUST call destroy_dictionary to free all the needed memory
 */
tll_dictionary_t* create_dictionary(void);

/**
 * Returns the count of current defined elements with a valid key
 */
int get_size(const tll_dictionary_t* dict);

/**
 * Adds the given item to the given dictionary at the key's position if there is nothing there already
 */
void add_item(tll_dictionary_t* dict, char* key, void* item);

/**
 * Returns the item at the key's position
 */
void* get_item(const tll_dictionary_t* dict, char* key);

/**
 * Inserts the given item into the key's position, loosing the item that was before there
 *
 * @note If the key was never used before, this function does nothing
 */
void replace_item(tll_dictionary_t* dict, char* key, void* new_item);

/**
 * Deletes the item at the key's position, if the key was never used it does nothing
 */
void delete_item(tll_dictionary_t* dict, char* key);

/**
 * Frees all the memory used by the given dictionary
 */
void destroy_dictionary(tll_dictionary_t* dict);

#endif
