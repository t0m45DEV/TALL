#include "tll_code_chunk.h"

#include "tll_memory.h"
#include "tll_value.h"

#include <stdint.h>
#include <stdlib.h>

void init_code_chunk(tll_code_chunk* code_chunk)
{
    code_chunk->count = 0;
    code_chunk->capacity = 0;
    code_chunk->code = NULL;
    init_value_array(&code_chunk->constants);
}

void free_code_chunk(tll_code_chunk* code_chunk)
{
    FREE_ARRAY(uint8_t, code_chunk->code, code_chunk->capacity);
    free_value_array(&code_chunk->constants);
    init_code_chunk(code_chunk);
}

void write_code_chunk(tll_code_chunk *code_chunk, uint8_t byte)
{
    if (code_chunk->count + 1 > code_chunk->capacity)
    {
        int old_capacity = code_chunk->capacity;
        code_chunk->capacity = GROW_CAPACITY(old_capacity);
        code_chunk->code = GROW_ARRAY(uint8_t, code_chunk->code, old_capacity, code_chunk->capacity);
    }
    code_chunk->code[code_chunk->count] = byte;
    code_chunk->count++;
}

int add_constant(tll_code_chunk *code_chunk, tll_value constant)
{
    write_value_array(&code_chunk->constants, constant);
    return code_chunk->constants.count - 1;
}

