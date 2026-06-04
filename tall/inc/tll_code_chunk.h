#ifndef _TLL_CODE_CHUNK_H
#define _TLL_CODE_CHUNK_H

#include "tll_common.h"

/**
 * The operations the TALL VM will be running. An byte-sized-ASM-like operations collection.
 */
typedef enum {
    OP_RETURN,
} tll_opcode;

/**
 * A representation for a collection of instructions the TALL VM can read and run.
 */
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
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
void write_code_chunk(tll_code_chunk* code_chunk, uint8_t byte);

#endif
