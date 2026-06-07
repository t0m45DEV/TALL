#ifndef _TLL_COMPILER_H
#define _TLL_COMPILER_H

#include "tll_code_chunk.h"

#include <stdbool.h>

/**
 * Parse the given source_code, checking for compile errors, generates bytecode for the TALL VM, then saves it on the given code_chunk. Returns true if the compilation went succesful.
 */
bool compile_code(const char* source_code, tll_code_chunk* code_chunk);

#endif
