#include "tll_vm.h"

#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"

#include <stdio.h>
#include <stdint.h>

tll_vm VM;

static void reset_stack(void)
{
    VM.stack_top = VM.stack;
}

static tll_interpret_result run_VM_code(void)
{
    #define READ_BYTE() (*VM.ip++)
    #define READ_CONSTANT() (VM.code_chunk->constants.values[READ_BYTE()])

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

        switch (instruction = READ_BYTE())
        {
            case OP_CONSTANT:
                push(READ_CONSTANT());
                break;

            case OP_RETURN:
                print_value(pop());
                printf("\n");
                return TLL_INTERPRET_OK;
        }
    }
    #undef READ_CONSTANT
    #undef READ_BYTE
}

void init_VM(void)
{
    reset_stack();
}

void free_VM(void)
{

}

tll_interpret_result interpret_code(tll_code_chunk* code_chunk)
{
    VM.code_chunk = code_chunk;
    VM.ip = VM.code_chunk->code;
    return run_VM_code();
}

void push(tll_value value)
{
    *VM.stack_top = value;
    VM.stack_top++;
}

tll_value pop(void)
{
    VM.stack_top--;
    return *VM.stack_top;
}

