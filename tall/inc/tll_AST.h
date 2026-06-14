#ifndef _TLL_AST_H
#define _TLL_AST_H

#include "tll_value.h"
#include "tll_scanner.h"
#include <stdint.h>

typedef enum {
    AST_LITERAL,
    AST_UNARY,
    AST_BINARY,
    AST_GROUPING
} tll_AST_type;

typedef struct tll_AST {
    tll_AST_type type;
    union {
        struct {
            tll_value value;
        } literal;

        struct {
            uint8_t op;
            struct tll_AST* operand;
        } unary;

        struct {
            uint8_t op;
            struct tll_AST* left;
            struct tll_AST* right;
        } binary;

        struct {
            struct tll_AST* expression;
        } grouping;
    } as;
} tll_AST;

/**
 * Generate an Abstract Syntax Tree from the given tokens.
 */
tll_AST* create_AST(tll_token* tokens);

/**
 * Frees the memory used by the AST.
 */
void free_AST(tll_AST* AST);

/**
 * Prints out in a "nice" way the given AST.
 */
void print_AST(const tll_AST* AST, const char* name);

#endif
