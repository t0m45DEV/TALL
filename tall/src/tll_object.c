#include "tll_object.h"

#include "tll_code_chunk.h"
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

/**
 * Print the given TLL string to standar output.
 */
static inline void print_string(const tll_string string);

/**
 * Print the given TLL function to standar output.
 */
static void print_function(const tll_function function);

tll_string* take_string(char* chars, int length)
{
    return allocate_string(chars, length);
}

tll_string* copy_string(const char* chars, int length)
{
    char* heap_chars = ALLOCATE_ARRAY(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';

    return allocate_string(heap_chars, length);
}

tll_function* new_function(void)
{
    tll_function* function = ALLOCATE_OBJ(tll_function, OBJ_FUNCTION);

    function->arity = 0;
    function->arguments_types = NULL;
    function->name = NULL;
    function->return_type = VAL_NULL;
    init_code_chunk(&function->code_chunk);

    return function;
}

tll_native_function* new_native_function(native_function function, int parameter_count, const tll_value parameters[])
{
    tll_native_function* native = ALLOCATE_OBJ(tll_native_function, OBJ_NATIVE);
    native->function = function;
    native->parameter_count = parameter_count;

    if (parameter_count > 0)
    {
        native->parameters = ALLOCATE_ARRAY(tll_value, parameter_count);
    }
    else
    {
        native->parameters = NULL;
    }

    for (int i = 0; i < parameter_count; i++)
    {
        native->parameters[i] = parameters[i];
    }
    return native;
}

bool check_types(const tll_native_function* native, int arg_count, const tll_value* args)
{
    if (native->parameter_count != arg_count)
    {
        return false;
    }
    for (int i = 0; i < arg_count; i++)
    {
        if (!same_type(native->parameters[i], args[i]))
        {
            return false;
        }
    }
    return true;
}

void free_object(tll_obj* object)
{
    switch (object->type)
    {
        case OBJ_STRING:
        {
            FREE_ARRAY(char, ((tll_string*) object)->chars, ((tll_string*) object)->length + 1);
            FREE_POINTER(tll_string, (tll_string*) object);
            break;
        }
        case OBJ_FUNCTION:
        {
            tll_function* function = (tll_function*) object;

            free_code_chunk(&function->code_chunk);
            FREE_ARRAY(tll_value_type, function->arguments_types, function->arity);
            FREE_POINTER(tll_function, function);
            break;
        }
        case OBJ_NATIVE:
        {
            tll_native_function* native = (tll_native_function*) object;

            FREE_ARRAY(tll_value, native->parameters, native->parameter_count);
            FREE_POINTER(tll_native_function, object);
            break;
        }
    }
}

void print_object(tll_value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
        {
            print_string(*AS_TLL_STRING(value));
            break;
        }
        case OBJ_FUNCTION:
        {
            print_function(*AS_TLL_FUNCTION(value));
            break;
        }
        case OBJ_NATIVE:
        {
            printf("<native func>");
            break;
        }
    }
}

void init_object_pool(void)
{
    object_pool = NULL;
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

    return string;
}

static inline void print_string(const tll_string string)
{
    printf("%.*s", string.length, string.chars);
}

static void print_function(const tll_function function)
{
    if (function.name == NULL)
    {
        printf("<script>");
    }
    else
    {
        printf("<func ");
        print_string(*function.name);
        printf(">");
    }
}

