#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    tll_code_chunk chunk;
    init_code_chunk(&chunk);

    int constant = add_constant(&chunk, (tll_value) 1.2);

    write_code_chunk(&chunk, OP_CONSTANT, 123);
    write_code_chunk(&chunk, constant, 123);
    write_code_chunk(&chunk, OP_RETURN, 123);

    disassemble_code_chunk(&chunk, "Test chunk");

    free_code_chunk(&chunk);

    exit(EXIT_SUCCESS);
}
