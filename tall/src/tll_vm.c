#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_dictionary.h"
#include "tll_standard_library.h"
#include "tll_value.h"
#include "tll_object.h"
#include "tll_memory.h"
#include "tll_compiler.h"
#include "tll_flags.h"

#include <stddef.h>
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
 * Returns the current byte being seen by the given frame, and advances it immediately.
 */
static inline uint8_t read_byte(tll_call_frame* frame);

/**
 * Returns the current two bytes being seen by the given frame in the form of a 16 bits integer, and advances it immediately.
 */
static inline uint16_t read_short(tll_call_frame* frame);

/**
 * Reads the short currently being seen by the given frame, advances it immediately, and then returns the constant from the constant pool at that short position.
 */
static inline tll_value read_constant(tll_call_frame* frame);

/**
 * Reads the string pointer currently being seen by the given frame, advances it immediately, and then returns that pointer.
 */
static inline tll_string* read_string(tll_call_frame* frame);

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
    init_dictionary(&VM.global_vars);
    init_dictionary(&VM.global_consts);
    init_object_pool();
    reset_stack();
    load_tll_std_lib();
}

void free_VM(void)
{
    free_dictionary(&VM.global_vars);
    free_dictionary(&VM.global_consts);
    free_object_pool();
    reset_stack();
}

tll_interpret_result interpret_code(const char* source_code)
{
    tll_code_chunk code_chunk;
    init_code_chunk(&code_chunk);

    tll_function* compiled_function = compile_code(source_code, &code_chunk);

    if (compiled_function == NULL)
    {
        free_code_chunk(&code_chunk);
        return TLL_INTERPRET_COMPILE_ERROR;
    }
    push(AS_TLL_OBJ(compiled_function));

    if (VM.frame_count + 1 > VM.frame_capacity)
    {
        int old_capacity = VM.frame_capacity;
        VM.frame_capacity = GROW_CAPACITY(VM.frame_capacity);
        VM.frames = GROW_ARRAY(tll_call_frame, VM.frames, old_capacity, VM.frame_capacity);
    }
    tll_call_frame* frame = &VM.frames[VM.frame_count++];

    frame->function = compiled_function;
    frame->ip = compiled_function->code_chunk.code;
    frame->slots = VM.stack;

    return run_VM_code();
}

void push(tll_value value)
{
    if (VM.stack_top - VM.stack >= VM.stack_capacity)
    {
        int top_offset = VM.stack_top - VM.stack;
        int old_capacity = VM.stack_capacity;
        tll_value* old_stack = VM.stack;

        VM.stack_capacity = GROW_CAPACITY(old_capacity);
        VM.stack = GROW_ARRAY(tll_value, VM.stack, old_capacity, VM.stack_capacity);
        VM.stack_top = VM.stack + top_offset;

        // If a new block of memory has been allocated, we need to fix slots.
        if (VM.stack != old_stack)
        {
            ptrdiff_t delta = VM.stack - old_stack;

            for (int i = 0; i < VM.frame_count; i++)
            {
                VM.frames[i].slots += delta;
            }
        }
    }
    *VM.stack_top = value;
    VM.stack_top++;
}

tll_value pop(void)
{
    VM.stack_top--;
    return *VM.stack_top;
}

void define_native_function(const char* name, int name_len, native_function function, int arg_count, const tll_value parameters[])
{
    push(AS_TLL_OBJ(copy_string(name, name_len)));
    push(AS_TLL_OBJ(new_native_function(function, arg_count, parameters)));
    set_to_dictionary(&VM.global_consts, AS_TLL_STRING(VM.stack[0]), VM.stack[1]);
    pop();
    pop();
}

static tll_interpret_result run_VM_code(void)
{
    tll_call_frame* frame = &VM.frames[VM.frame_count - 1];

    while (true)
    {
        if (is_trace_flag())
        {
            printf("          ");
            for (tll_value* slot = VM.stack; slot < VM.stack_top; slot++)
            {
                printf("[ ");
                print_value(*slot);
                printf(" ]");
            }
            printf("\n");
            disassemble_instruction(&frame->function->code_chunk, (int) (frame->ip - frame->function->code_chunk.code));
        }
        uint8_t instruction = read_byte(frame);

        switch (instruction)
        {
            case OP_CONSTANT:
            {
                push(read_constant(frame));
                break;
            }
            case OP_NULL:
            {
                push(AS_TLL_NULL);
                break;
            }
            case OP_TRUE:
            {
                push(AS_TLL_BOOL(true));
                break;
            }
            case OP_FALSE:
            {
                push(AS_TLL_BOOL(false));
                break;
            }
            case OP_POP:
            {
                pop();
                break;
            }
            case OP_DEF_GLOBAL_CONST:
            {
                set_to_dictionary(&VM.global_consts, read_string(frame), peek(0));
                pop();
                break;
            }
            case OP_DEF_GLOBAL_VAR:
            {
                set_to_dictionary(&VM.global_vars, read_string(frame), peek(0));
                pop();
                break;
            }
            case OP_GET_GLOBAL:
            {
                tll_value value;
                tll_string* name = read_string(frame);

                if (get_from_dictionary(&VM.global_vars, name, &value))
                {
                    push(value);
                    break;
                }
                else if (get_from_dictionary(&VM.global_consts, name, &value))
                {
                    push(value);
                    break;
                }
                else
                {
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break; // Unreachable.
            }
            case OP_SET_GLOBAL:
            {
                tll_value value;
                tll_string* name = read_string(frame);

                if (!get_from_dictionary(&VM.global_vars, name, &value))
                {
                    if (get_from_dictionary(&VM.global_consts, name, &value))
                    {
                        runtime_error("Trying to redefine constant '%s'.", name->chars);
                    }
                    else
                    {
                        runtime_error("Cannot update the undefined variable '%s'.", name->chars);
                    }
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                set_to_dictionary(&VM.global_vars, name, peek(0));
                break;
            }
            case OP_GET_LOCAL:
            {
                push(frame->slots[read_byte(frame)]);
                break;
            }
            case OP_SET_LOCAL:
            {
                frame->slots[read_byte(frame)] = peek(0);
                break;
            }
            case OP_EQUAL:
            {
                if (peek(0).type != peek(1).type)
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();
                push(AS_TLL_BOOL(are_equals(a, b)));
                break;
            }
            case OP_NOT_EQUAL:
            {
                if (peek(0).type != peek(1).type)
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();
                push(AS_TLL_BOOL(!are_equals(a, b)));
               break;
            }
            case OP_GREATER:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_GREATER_EQUAL:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_LESS:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_LESS_EQUAL:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must have the same type to be compared.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_ADD:
            {
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
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_SUBSTRACT:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_MULTIPLY:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_DIVIDE:
            {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                tll_value b = pop();
                tll_value a = pop();

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
            }
            case OP_NOT:
            {
                if (!IS_BOOL(peek(0)))
                {
                    runtime_error("Operand must be a boolean.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                push(AS_TLL_BOOL(!AS_C_BOOL(pop())));
                break;
            }
            case OP_NEGATE:
            {
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
            }
            case OP_JMP_BACK:
            {
                frame->ip -= read_short(frame);
                break;
            }
            case OP_JMP_AHEAD:
            {
                frame->ip += read_short(frame);
                break;
            }
            case OP_JMP_IF_FALSE:
            {
                uint16_t offset = read_short(frame);

                if (are_equals(AS_TLL_BOOL(false), peek(0)))
                {
                    frame->ip += offset;
                }
                break;
            }
            case OP_JMP_IF_TRUE:
            {
                uint16_t offset = read_short(frame);

                if (are_equals(AS_TLL_BOOL(true), peek(0)))
                {
                    frame->ip += offset;
                }
                break;
            }
            case OP_CALL:
            {
                uint16_t arg_count = read_short(frame);
                tll_value func = peek(arg_count);

                if (func.type == VAL_OBJ)
                {
                    switch (func.as.obj->type)
                    {
                        case OBJ_FUNCTION:
                        {
                            tll_function* callable = AS_TLL_FUNCTION(func);

                            if (callable->arity != arg_count)
                            {
                                runtime_error("Wrong argument count for a function call, expected %i but %i were given.", callable->arity, arg_count);
                                return TLL_INTERPRET_RUNTIME_ERROR;
                            }
                            else
                            {
                                if (VM.frame_count + 1 > VM.frame_capacity)
                                {
                                    int old_capacity = VM.frame_capacity;
                                    VM.frame_capacity = GROW_CAPACITY(VM.frame_capacity);
                                    VM.frames = GROW_ARRAY(tll_call_frame, VM.frames, old_capacity, VM.frame_capacity);
                                }
                                tll_call_frame* callee_frame = &VM.frames[VM.frame_count++];

                                callee_frame->function = callable;
                                callee_frame->ip = callable->code_chunk.code;
                                callee_frame->slots = VM.stack_top - arg_count - 1;

                                frame = &VM.frames[VM.frame_count - 1];
                            }
                            break;
                        }
                        case OBJ_NATIVE:
                        {
                            tll_native_function* native = AS_TLL_NATIVE(func);

                            if (!check_types(native, arg_count, VM.stack_top - arg_count))
                            {
                                runtime_error("Wrong argument type for a native function.");
                                return TLL_INTERPRET_RUNTIME_ERROR;
                            }
                            tll_value result = native->function(arg_count, VM.stack_top - arg_count);
                            VM.stack_top -= arg_count + 1;
                            push(result);
                            break;
                        }
                        default:
                        {
                            runtime_error("Only functions can be called.");
                            return TLL_INTERPRET_RUNTIME_ERROR;
                        }
                    }
                }
                else
                {
                    runtime_error("Tried to call a non callable data type.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_RETURN:
            {
                tll_value result = pop();

                VM.frame_count--;
                VM.stack_top = frame->slots;

                if (VM.frame_count == 0)
                {
                    return TLL_INTERPRET_OK;
                }
                else
                {
                    push(result);
                    frame = &VM.frames[VM.frame_count - 1];
                }
                break;
            }
            default:
            {
                runtime_error("Unkown instruction with op code %i at line %i.", instruction, frame->ip);
                return TLL_INTERPRET_RUNTIME_ERROR;
            }
        }
    }
}

static inline uint8_t read_byte(tll_call_frame* frame)
{
    return *frame->ip++;
}

static inline uint16_t read_short(tll_call_frame* frame)
{
    uint8_t high = read_byte(frame);
    uint8_t low = read_byte(frame);
    return (uint16_t) ((high << 8) | low);
}

static inline tll_value read_constant(tll_call_frame* frame)
{
    return frame->function->code_chunk.constants.values[read_short(frame)];
}

static inline tll_string* read_string(tll_call_frame* frame)
{
    return AS_TLL_STRING(read_constant(frame));
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
    tll_call_frame* frame = &VM.frames[VM.frame_count - 1];
    size_t instruction = frame->ip - frame->function->code_chunk.code - 1;
    int line = frame->function->code_chunk.lines[instruction];

    fprintf(stderr, "[line %i] Execution error: ", line);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    reset_stack();
}

static void reset_stack(void)
{
    FREE_ARRAY(tll_value, VM.stack, VM.stack_capacity);
    VM.stack_capacity = 0;
    VM.stack = NULL;
    VM.stack_top = NULL;

    FREE_ARRAY(tll_call_frame, VM.frames, VM.frame_capacity);
    VM.frames = NULL;
    VM.frame_count = 0;
    VM.frame_capacity = 0;

}

