#include "tll_debug.h"
#include "tll_code_chunk.h"

#include <stdint.h>
#include <stdio.h>

static int simple_instruction(const char* op_name, int offset)
{
    printf("%s\n", op_name);
    return offset + 1;
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
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unkown opcode %d\n", instruction);
            return offset + 1;
    }
}

