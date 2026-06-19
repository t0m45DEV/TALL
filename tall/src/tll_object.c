#include "tll_object.h"

#include "tll_dictionary.h"
#include "tll_memory.h"
#include "tll_value.h"

#include <stdio.h>
#include <string.h>

#define ALLOCATE_OBJ(type, object_type) ((type*) allocate_object(sizeof(type), object_type))

/**
 * A linked list object collection to track the objects being allocated at the heap.
 */
typedef struct tll_object_node {
    tll_obj* object;
    struct tll_object_node* next;
} tll_object_node;

// The dynamically allocated TLL objects.
tll_object_node* object_pool;

// A set of all the strings being used by the VM.
tll_dictionary strings;

/**
 * Adds the given TLL object to the object pool.
 */
static void write_object_pool(tll_obj* object);

/**
 * Returns a pointer with the memory allocated to hold a TLL object of the given type.
 */
static tll_obj* allocate_object(size_t size, tll_obj_type type);

/**
 * Returns a pointer with the memory needed for a TLL string object with the given string of size length.
 */
static tll_string* allocate_string(char* chars, int length);

tll_string* take_string(char* chars, int length)
{
    tll_string* interned = dictionary_find_string(&strings, chars, length);

    // If this is an already created string, we use the same object.
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocate_string(chars, length);
}

tll_string* copy_string(const char* chars, int length)
{
    tll_string* interned = dictionary_find_string(&strings, chars, length);

    if (interned != NULL)
    {
        return interned;
    }
    char* heap_chars = ALLOCATE_ARRAY(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';

    return allocate_string(heap_chars, length);
}

void free_object(tll_obj* object)
{
    switch (object->type)
    {
        case OBJ_STRING:
            FREE_ARRAY(char, ((tll_string*) object)->chars, ((tll_string*) object)->length + 1);
            FREE_POINTER(tll_string, (tll_string*) object);
            break;
    }
}

void print_object(tll_value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_C_STRING(value));
            break;
    }
}

void init_object_pool(void)
{
    object_pool = NULL;
    init_dictionary(&strings);
}

void free_object_pool(void)
{
    tll_object_node* current = object_pool;

    while (current != NULL)
    {
        tll_object_node* next = current->next;
        free_object(current->object);
        FREE_POINTER(tll_object_node, current);
        current = next;
    }
    free_dictionary(&strings);
    init_object_pool();
}

static void write_object_pool(tll_obj* object)
{
    tll_object_node* node = ALLOCATE_POINTER(tll_object_node);
    node->object = object;
    node->next = object_pool;
    object_pool = node;
}

static tll_obj* allocate_object(size_t size, tll_obj_type obj_type)
{
    tll_obj* object = (tll_obj*) reallocate(NULL, 0, size);
    object->type = obj_type;
    write_object_pool(object);
    return object;
}

static tll_string* allocate_string(char* chars, int length)
{
    tll_string* string = ALLOCATE_OBJ(tll_string, OBJ_STRING);
    string->length = length;
    string->chars = chars;

    set_to_dictionary(&strings, string, AS_TLL_NULL);
    return string;
}

