#include "tll_code_chunk.h"
#include "tll_debug.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    tll_code_chunk chunk;
    init_code_chunk(&chunk);
    write_code_chunk(&chunk, OP_RETURN);

    disassemble_code_chunk(&chunk, "Test chunk");

    free_code_chunk(&chunk);

    exit(EXIT_SUCCESS);
}
