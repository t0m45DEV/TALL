#include "tll_debug.h"

#include "tll_value.h"
#include "tll_code_chunk.h"

#include <stdint.h>
#include <stdio.h>

static int simple_instruction(const char* op_name, int offset)
{
    printf("%s\n", op_name);
    return offset + 1;
}

static int constant_instruction(const char* op_name, tll_code_chunk* code_chunk, int offset)
{
    uint8_t constant = code_chunk->code[offset + 1];
    printf("%-16s %4d '", op_name, constant);
    print_value(code_chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

void disassemble_code_chunk(tll_code_chunk* code_chunk, const char* chunk_name)
{
    printf("=== %s ===\n", chunk_name);

    int offset = 0;
    while (offset < code_chunk->count)
    {
        offset = disassemble_instruction(code_chunk, offset);
    }
}

int disassemble_instruction(tll_code_chunk* code_chunk, int offset)
{
    printf("%04d ", offset);

    uint8_t instruction = code_chunk->code[offset];

    switch (instruction)
    {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", code_chunk, offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unkown opcode %d\n", instruction);
            return offset + 1;
    }
}

