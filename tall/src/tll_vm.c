#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_dictionary.h"
#include "tll_value.h"
#include "tll_object.h"
#include "tll_memory.h"
#include "tll_compiler.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

// The TALL VM.
tll_vm VM;

/**
 * Run the bytecode inside the VM.
 */
static tll_interpret_result run_VM_code(void);

/**
 * Returns the current byte being seen by the VM, and advances it immediately.
 */
static inline uint8_t read_byte(void);

/**
 * Returns the current two bytes being seen by the VM in the form of a 16 bits integer, and advances the VM immediately.
 */
static inline uint16_t read_short(void);

/**
 * Reads the short currently being seen by the VM, advances the VM immediately, and then returns the constant from the constant pool at that short position.
 */
static inline tll_value read_constant(void);

/**
 * Reads the string pointer currently being seen by the VM, advances the VM immediately, and then returns that pointer.
 */
static inline tll_string* read_string(void);

/**
 * Returns the value at distance from the current top of the VM stack.
 */
static inline tll_value peek(int distance);

/**
 * Pops the two values at the top of the stack and, assumming they are strings, it push back up the concatenation.
 */
static void concatenate(void);

/**
 * Given a message, it prints out an error with the location of the detected bug.
 */
static void runtime_error(const char* format, ...);

/**
 * Deletes all the contents of the current stack, make it blank.
 */
static void reset_stack(void);

void init_VM(void)
{
    init_dictionary(&VM.globals);
    init_object_pool();
    reset_stack();
}

void free_VM(void)
{
    free_dictionary(&VM.globals);
    free_object_pool();
    reset_stack();
}

tll_interpret_result interpret_code(const char* source_code)
{
    tll_code_chunk code_chunk;
    init_code_chunk(&code_chunk);

    if (!compile_code(source_code, &code_chunk))
    {
        free_code_chunk(&code_chunk);
        return TLL_INTERPRET_COMPILE_ERROR;
    }
    VM.code_chunk = &code_chunk;
    VM.ip = VM.code_chunk->code;

    tll_interpret_result result = run_VM_code();

    free_code_chunk(&code_chunk);
    return result;
}

void push(tll_value value)
{
    if (VM.stack_top - VM.stack >= VM.stack_capacity)
    {
        int top_offset = VM.stack_top - VM.stack;
        int old_capacity = VM.stack_capacity;
        VM.stack_capacity = GROW_CAPACITY(old_capacity);
        VM.stack = GROW_ARRAY(tll_value, VM.stack, old_capacity, VM.stack_capacity);
        VM.stack_top = VM.stack + top_offset;
    }
    *VM.stack_top = value;
    VM.stack_top++;
}

tll_value pop(void)
{
    VM.stack_top--;
    return *VM.stack_top;
}

static tll_interpret_result run_VM_code(void)
{
    while (true)
    {
        #ifdef TLL_DEBUG_TRACE_EXECUTION
            printf("          ");
            for (tll_value* slot = VM.stack; slot < VM.stack_top; slot++)
            {
                printf("[ ");
                print_value(*slot);
                printf(" ]");
            }
            printf("\n");
            disassemble_instruction(VM.code_chunk, (int) (VM.ip - VM.code_chunk->code));
        #endif

        uint8_t instruction = read_byte();

        tll_value a, b;   // For the binary operations.
        tll_string* name; // For variable or function names.

        switch (instruction)
        {
            case OP_CONSTANT:
                push(read_constant());
                break;

            case OP_NULL:
                push(AS_TLL_NULL);
                break;

            case OP_TRUE:
                push(AS_TLL_BOOL(true));
                break;

            case OP_FALSE:
                push(AS_TLL_BOOL(false));
                break;

            case OP_POP:
                pop();
                break;

            case OP_DEFINE_GLOBAL:
                set_to_dictionary(&VM.globals, read_string(), peek(0));
                pop();
                break;

            case OP_GET_GLOBAL:
                name = read_string();

                if (!get_from_dictionary(&VM.globals, name, &a))
                {
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                push(a);
                break;

            case OP_EQUAL:

                if (peek(0).type != peek(1).type)
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();
                push(AS_TLL_BOOL(are_equals(a, b)));
                break;

            case OP_NOT_EQUAL:

                if (peek(0).type != peek(1).type)
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();
                push(AS_TLL_BOOL(!are_equals(a, b)));
                break;

            case OP_GREATER:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(a) && IS_INT(b))
                {
                    push(AS_TLL_BOOL(AS_C_INT(a) > AS_C_INT(b)));
                }
                else if (IS_FLOAT(a) && IS_FLOAT(b))
                {
                    push(AS_TLL_BOOL(AS_C_FLOAT(a) > AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_GREATER_EQUAL:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(a) && IS_INT(b))
                {
                    push(AS_TLL_BOOL(AS_C_INT(a) >= AS_C_INT(b)));
                }
                else if (IS_FLOAT(a) && IS_FLOAT(b))
                {
                    push(AS_TLL_BOOL(AS_C_FLOAT(a) >= AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_LESS:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(a) && IS_INT(b))
                {
                    push(AS_TLL_BOOL(AS_C_INT(a) < AS_C_INT(b)));
                }
                else if (IS_FLOAT(a) && IS_FLOAT(b))
                {
                    push(AS_TLL_BOOL(AS_C_FLOAT(a) < AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_LESS_EQUAL:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(a) && IS_INT(b))
                {
                    push(AS_TLL_BOOL(AS_C_INT(a) <= AS_C_INT(b)));
                }
                else if (IS_FLOAT(a) && IS_FLOAT(b))
                {
                    push(AS_TLL_BOOL(AS_C_FLOAT(a) <= AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_ADD:

                if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
                {
                    concatenate();
                    break;
                }
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    if (!IS_STRING(peek(0)) && !IS_STRING(peek(1)))
                    {
                        runtime_error("Operands must be either 'int', 'float' or 'string'.");
                        return TLL_INTERPRET_RUNTIME_ERROR;
                    }
                    else
                    {
                        runtime_error("Concatenation works when both operands are 'string'.");
                        return TLL_INTERPRET_RUNTIME_ERROR;
                    }
                }
                b = pop();
                a = pop();

                if (IS_INT(b) && IS_INT(a))
                {
                    push(AS_TLL_INT(AS_C_INT(a) + AS_C_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(AS_TLL_FLOAT(AS_C_FLOAT(a) + AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_SUBSTRACT:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(b) && IS_INT(a))
                {
                    push(AS_TLL_INT(AS_C_INT(a) - AS_C_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(AS_TLL_FLOAT(AS_C_FLOAT(a) - AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_MULTIPLY:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(b) && IS_INT(a))
                {
                    push(AS_TLL_INT(AS_C_INT(a) * AS_C_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(AS_TLL_FLOAT(AS_C_FLOAT(a) * AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_DIVIDE:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(b) && IS_INT(a))
                {
                    push(AS_TLL_INT((int) (AS_C_INT(a) / AS_C_INT(b))));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(AS_TLL_FLOAT(AS_C_FLOAT(a) / AS_C_FLOAT(b)));
                }
                else
                {
                    runtime_error("Operands must be the same numerical type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_NOT:
                if (!IS_BOOL(peek(0)))
                {
                    runtime_error("Operand must be a boolean.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                push(AS_TLL_BOOL(!AS_C_BOOL(pop())));
                break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0)))
                {
                    runtime_error("Operand must be a number.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                if (IS_INT(peek(0)))
                {
                    push(AS_TLL_INT(-AS_C_INT(pop())));
                }
                else if (IS_FLOAT(peek(0)))
                {
                    push(AS_TLL_FLOAT(-AS_C_FLOAT(pop())));
                }
                break;

            case OP_RETURN:
                print_value(pop());
                printf("\n");
                return TLL_INTERPRET_OK;
        }
    }
}

static inline uint8_t read_byte(void)
{
    return *VM.ip++;
}

static inline uint16_t read_short(void)
{
    uint8_t high = read_byte();
    uint8_t low = read_byte();
    return (uint16_t) ((high << 8) | low);
}

static inline tll_value read_constant(void)
{
    return VM.code_chunk->constants.values[read_short()];
}

static inline tll_string* read_string(void)
{
    return AS_TLL_STRING(read_constant());
}

static inline tll_value peek(int distance)
{
    return VM.stack_top[-1 - distance];
}

static void concatenate(void)
{
    tll_string* b = AS_TLL_STRING(pop());
    tll_string* a = AS_TLL_STRING(pop());

    int length = a->length + b->length;
    char* chars = ALLOCATE_ARRAY(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    tll_string* result = take_string(chars, length);
    push(AS_TLL_OBJ(result));
}

static void runtime_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = VM.ip - VM.code_chunk->code - 1;
    int line = VM.code_chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

static void reset_stack(void)
{
    FREE_ARRAY(tll_value, VM.stack, VM.stack_capacity);
    VM.stack_capacity = 0;
    VM.stack = NULL;
    VM.stack_top = NULL;
}

