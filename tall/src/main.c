#include "tll_code_chunk.h"
#include "tll_debug.h"
#include "tll_value.h"
#include "tll_vm.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    init_VM();

    tll_code_chunk chunk;
    init_code_chunk(&chunk);

    int constant = add_constant(&chunk, (tll_value) 1.2);
    write_code_chunk(&chunk, OP_CONSTANT, 123);
    write_code_chunk(&chunk, constant, 123);

    constant = add_constant(&chunk, (tll_value) 3.4);
    write_code_chunk(&chunk, OP_CONSTANT, 123);
    write_code_chunk(&chunk, constant, 123);

    write_code_chunk(&chunk, OP_ADD, 123);

    constant = add_constant(&chunk, (tll_value) 5.6);
    write_code_chunk(&chunk, OP_CONSTANT, 123);
    write_code_chunk(&chunk, constant, 123);

    write_code_chunk(&chunk, OP_DIVIDE, 123);
    write_code_chunk(&chunk, OP_NEGATE, 123);
    write_code_chunk(&chunk, OP_RETURN, 123);

    interpret_code(&chunk);
    free_code_chunk(&chunk);

    free_VM();
    exit(EXIT_SUCCESS);
}

