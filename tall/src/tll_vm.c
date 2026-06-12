#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"
#include "tll_memory.h"
#include "tll_compiler.h"

#include <stdio.h>
#include <stdint.h>

tll_vm VM;

static void reset_stack(void)
{
    FREE_ARRAY(tll_value, VM.stack, VM.stack_capacity);
    VM.stack_capacity = 0;
    VM.stack = NULL;
    VM.stack_top = NULL;
}

static tll_interpret_result run_VM_code(void)
{
    #define READ_BYTE() (*VM.ip++)
    #define READ_SHORT() ((uint8_t) ((READ_BYTE() << 8) | READ_BYTE()))
    #define READ_CONSTANT() (VM.code_chunk->constants.values[READ_SHORT()])

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

        switch (instruction = READ_BYTE())
        {
            case OP_CONSTANT:
                push(READ_CONSTANT());
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
    #undef READ_CONSTANT
    #undef READ_SHORT
    #undef READ_BYTE
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

