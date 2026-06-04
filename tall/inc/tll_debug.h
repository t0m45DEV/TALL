#ifndef _TLL_DEBUG_H
#define _TLL_DEBUG_H

#include "tll_code_chunk.h"

/**
 * Prints out to the standard output all the instructions of the given code_chunk, also adds a header with the given chunk_name.
 */
void disassemble_code_chunk(tll_code_chunk* code_chunk, const char* chunk_name);

/**
 * Prints out to the standard output the instruction located at the given offset inside the given code chunk, then returns the offset for the next instruction.
 */
int disassemble_instruction(tll_code_chunk* code_chunk, int offset);

#endif
