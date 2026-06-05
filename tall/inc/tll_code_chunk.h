#ifndef _TLL_CODE_CHUNK_H
#define _TLL_CODE_CHUNK_H

#include "tll_common.h"
#include "tll_value.h"

/**
 * The operations the TALL VM will be running. An byte-sized-ASM-like operations collection.
 */
typedef enum {
    OP_CONSTANT,
    OP_NEGATE,
    OP_RETURN,
} tll_opcode;

/**
 * A representation for a collection of data and instructions the TALL VM can read and run.
 */
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    tll_value_array constants;
} tll_code_chunk;

/**
 * Initialize the given chunk.
 */
void init_code_chunk(tll_code_chunk* code_chunk);

/**
 * Frees the memory being used by the given code chunk.
 */
void free_code_chunk(tll_code_chunk* code_chunk);

/**
 * Write the given byte into the given chunk.
 */
void write_code_chunk(tll_code_chunk* code_chunk, uint8_t byte, int line);

/**
 * Writes the given constant value to the given code_chunk. Returns the index where the constant was appended.
 */
int add_constant(tll_code_chunk* code_chunk, tll_value constant);

#endif
