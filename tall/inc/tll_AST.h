#ifndef _TLL_AST_H
#define _TLL_AST_H

#include "tll_value.h"
#include "tll_scanner.h"
#include <stdint.h>

typedef enum {
    AST_ERROR,            // Error node for bad parsing.
    AST_LITERAL,          // A literal value.
    AST_UNARY,            // Unary operations, like '!' or '-'.
    AST_BINARY,           // Binary operations, like equalities, comparations and arithmetic stuff.
    AST_GROUPING,         // An expression between parenthesis.
    AST_PROGRAM,          // The root of every TALL script.
    AST_EXPR_STATEMENT,   // An expression used as a statement.
    AST_VAR_DECLARATION,  // A var declaration, something like 'var x : int = 10'.
    AST_VAR_ASSIGNMENT,   // A var assignment, for updating the value of an already defined variable.
    AST_VAR_NAME,         // When a variable read is made, like 'return a'.
    AST_RETURN,           // For returning a variable, an expression or nothing at all.
    AST_BLOCK,            // For code inside brackets: '{' code '}'.
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

        struct {
            struct tll_AST** statements;
            int count;
        } program;

        struct {
            struct tll_AST* expression;
        } expression_statement;

        struct {
            tll_string* name;
            tll_value_type type;
            struct tll_AST* expression;
        } var_declaration;

        struct {
            tll_string* name;
            struct tll_AST* expression;
        } var_assigment;

        struct {
            tll_string* name;
        } var_name;

        struct {
            struct tll_AST* expression;
        } return_statement;

        struct {
            struct tll_AST** declarations;
            int count;
        } block_assignment;
    } as;
} tll_AST;

/**
 * Generate an Abstract Syntax Tree from the given tokens.
 */
tll_AST* create_AST(tll_token* tokens);

/**
 * Frees the memory used by the AST.
 */
void end_AST(tll_AST* tree);

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
const tll_AST* get_error(const tll_AST* AST);

/**
 * Returns true if the given AST hasn't any type mismatch.
 */
bool type_checks(const tll_AST* AST);

/**
 * Returns the first node with a type mismatch of the given AST.
 *
 * Returns NULL if not found.
 */
const tll_AST* get_type_mismatch(const tll_AST* AST);

#endif
