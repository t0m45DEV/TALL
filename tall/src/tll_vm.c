#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"
#include "tll_memory.h"
#include "tll_compiler.h"

#include <stdio.h>
#include <stdint.h>

// The TALL VM.
tll_vm VM;

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

static void reset_stack(void)
{
    FREE_ARRAY(tll_value, VM.stack, VM.stack_capacity);
    VM.stack_capacity = 0;
    VM.stack = NULL;
    VM.stack_top = NULL;
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
                b = pop();
                a = pop();
                push(a + b);
                break;

            case OP_SUBSTRACT:
                b = pop();
                a = pop();
                push(a - b);
                break;

            case OP_MULTIPLY:
                b = pop();
                a = pop();
                push(a * b);
                break;

            case OP_DIVIDE:
                b = pop();
                a = pop();
                push(a / b);
                break;

            case OP_NEGATE:
                push(-pop());
                break;

            case OP_RETURN:
                print_value(pop());
                printf("\n");
                return TLL_INTERPRET_OK;
        }
    }
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

