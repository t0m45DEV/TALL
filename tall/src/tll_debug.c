#include "tll_debug.h"

#include "tll_value.h"
#include "tll_code_chunk.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

static int simple_instruction(const char* op_name, int offset)
{
    printf("%s\n", op_name);
    return offset + 1;
}

static int constant_instruction(const char* op_name, tll_code_chunk* code_chunk, int offset)
{
    uint8_t index_upper = code_chunk->code[offset + 1];
    uint8_t index_lower = code_chunk->code[offset + 2];

    uint16_t index = (index_upper << 8) | index_lower;

    printf("%-16s %4d '", op_name, index);

    tll_value value = code_chunk->constants.values[index];
    print_type(value);

    if (!IS_NULL(value))
    {
        printf(" ");
        print_value(code_chunk->constants.values[index]);
    }
    printf("'\n");
    return offset + 3;
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

    if (offset > 0 && code_chunk->lines[offset] == code_chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", code_chunk->lines[offset]);
    }
    uint8_t instruction = code_chunk->code[offset];

    switch (instruction)
    {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", code_chunk, offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBSTRACT:
            return simple_instruction("OP_SUBSTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unkown opcode %d\n", instruction);
            return offset + 1;
    }
}

