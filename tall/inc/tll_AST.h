#ifndef _TLL_AST_H
#define _TLL_AST_H

#include "tll_value.h"
#include "tll_scanner.h"
#include <stdint.h>
#include <threads.h>

typedef enum {
    AST_ERROR,
    AST_LITERAL,
    AST_UNARY,
    AST_BINARY,
    AST_GROUPING
} tll_AST_type;

typedef struct tll_AST {
    tll_AST_type type;
    int line;
    union {
        struct {
            const char* message;
        } error;

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
void end_AST(void);

/**
 * Prints out in a "nice" way the given AST.
 */
void print_AST(const tll_AST* AST, const char* name);

/**
 * Returns true if there is an AST_ERROR node inside the given AST.
 */
bool has_error(const tll_AST* AST);

/**
 * Returns the first node with type AST_ERROR it founds.
 *
 * Returns NULL otherwise.
 */
tll_AST* get_error(tll_AST* AST);

#endif
