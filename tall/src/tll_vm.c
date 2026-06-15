#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"
#include "tll_memory.h"
#include "tll_compiler.h"

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
inline static uint8_t read_byte(void);

/**
 * Returns the current two bytes being seen by the VM in the form of a 16 bits integer, and advances the VM immediately.
 */
inline static uint16_t read_short(void);

/**
 * Reads the short currently being seen by the VM, advances the VM immediately, and then returns the constant from the constant pool at that short position.
 */
inline static tll_value read_constant(void);

/**
 * Returns the value at distance from the current top of the VM stack.
 */
inline static tll_value peek(int distance);

/**
 * Given a message, it prints out an error with the location of the detected bug.
 */
static void runtime_error(const char* format, ...);

static void reset_stack(void)
{
    FREE_ARRAY(tll_value, VM.stack, VM.stack_capacity);
    VM.stack_capacity = 0;
    VM.stack = NULL;
    VM.stack_top = NULL;
}

void init_VM(void)
{
    reset_stack();
}

void free_VM(void)
{
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

        uint8_t instruction;

        tll_value a, b; // For the binary operations

        switch (instruction = read_byte())
        {
            case OP_CONSTANT:
                push(read_constant());
                break;

            case OP_ADD:

                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))
                {
                    runtime_error("Operands must be either 'int' or 'float'.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                b = pop();
                a = pop();

                if (IS_INT(b) && IS_INT(a))
                {
                    push(INT_VAL(AS_INT(a) + AS_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(FLOAT_VAL(AS_FLOAT(a) + AS_FLOAT(b)));
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
                    push(INT_VAL(AS_INT(a) - AS_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(FLOAT_VAL(AS_FLOAT(a) - AS_FLOAT(b)));
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
                    push(INT_VAL(AS_INT(a) * AS_INT(b)));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(FLOAT_VAL(AS_FLOAT(a) * AS_FLOAT(b)));
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
                    push(INT_VAL((int) (AS_INT(a) / AS_INT(b))));
                }
                else if (IS_FLOAT(b) && IS_FLOAT(a))
                {
                    push(FLOAT_VAL(AS_FLOAT(a) / AS_FLOAT(b)));
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
                push(BOOL_VAL(!AS_BOOL(pop())));
                break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0)))
                {
                    runtime_error("Operand must be a number.");
                    return TLL_INTERPRET_RUNTIME_ERROR;
                }
                if (IS_INT(peek(0)))
                {
                    push(INT_VAL(-AS_INT(pop())));
                }
                else if (IS_FLOAT(peek(0)))
                {
                    push(FLOAT_VAL(-AS_FLOAT(pop())));
                }
                break;

            case OP_RETURN:
                print_value(pop());
                printf("\n");
                return TLL_INTERPRET_OK;
        }
    }
}

inline static uint8_t read_byte(void)
{
    return *VM.ip++;
}

inline static uint16_t read_short(void)
{
    uint8_t high = read_byte();
    uint8_t low = read_byte();
    return (uint16_t) ((high << 8) | low);
}

inline static tll_value read_constant(void)
{
    return VM.code_chunk->constants.values[read_short()];
}

inline static tll_value peek(int distance)
{
    return VM.stack_top[-1 - distance];
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

