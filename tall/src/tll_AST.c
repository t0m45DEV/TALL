#include "tll_AST.h"

#include "tll_memory.h"
#include "tll_arena.h"
#include "tll_scanner.h"
#include "tll_value.h"

#include <stdlib.h>

#define EMPTY_NODE ((tll_AST) {AST_UNARY, -1, {.unary.op=0, .unary.operand=0}})

tll_memory_arena* first_arena;
tll_memory_arena* current_arena;

typedef struct {
    tll_token* current;
    tll_token* previous;
} tll_AST_parser;

tll_AST_parser AST_parser;

/**
 * Prints out in a nice way the given AST, using the given prefix for the next lines.
 *
 * is_last should be true if the given AST is a leafe.
 */
static void print_AST_recursive(const tll_AST* AST, const char* prefix, bool is_last);

/**
 * Prints out the given operation using the correct ASCII symbol.
 */
static void print_AST_operation(tll_token_type operation);

/**
 * Returns an empty node saved on the memory arena.
 */
static tll_AST* alloc_node(void);

/**
 * Advances the current parsing step for the AST.
 */
static void AST_advance(void);

/**
 * Returns true if the current token being parsed has the same type as the given one.
 */
inline static bool AST_check(tll_token_type type);

/**
 * Returns true if the current token being parsed has the same type as the given one, and if that's the case advances the AST.
 */
static bool AST_match(tll_token_type type);

/**
 * Generates an TALL expression AST from the given tokens collection that is an expression.
 */
static tll_AST* AST_expression(void);

/**
 * Parse the current token as either a sum or a substraction.
 */
static tll_AST* AST_term(void);

/**
 * Parse the current token as either a multiplication or a division.
 */
static tll_AST* AST_factor(void);

/**
 * Parse the current token as a negation.
 */
static tll_AST* AST_unary(void);

/**
 * Parse the current token as a value.
 */
static tll_AST* AST_primary(void);

tll_AST* create_AST(tll_token* tokens)
{
    first_arena = create_arena();
    current_arena = first_arena;

    AST_parser.current = tokens;
    AST_parser.previous = tokens;

    return AST_expression();
}

void end_AST(void)
{
    free_arena(first_arena);
    first_arena = NULL;
    current_arena = NULL;
}

void print_AST(const tll_AST* AST, const char* name)
{
    printf("%s\n", name);
    print_AST_recursive(AST, "", true);
}

static void print_AST_recursive(const tll_AST* AST, const char* prefix, bool is_last)
{
    printf("%s", prefix);

    if (is_last)
    {
        printf("└──");
    }
    else
    {
        printf("├──");
    }
    char child_prefix[256];

    if (is_last)
    {
        snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, "    ");
    }
    else
    {
        snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, "│   ");
    }

    switch (AST->type)
    {
        case AST_LITERAL:
            printf("LITERAL (");

            switch (AST->as.literal.value.type)
            {
                case VAL_NULL:
                    break;
                case VAL_BOOL:
                    printf("bool ");
                    break;
                case VAL_INT:
                    printf("int ");
                    break;
                case VAL_FLOAT:
                    printf("float ");
                    break;
            }
            print_value(AST->as.literal.value);
            printf(")\n");
            break;

        case AST_UNARY:
            printf("UNARY (");
            print_AST_operation(AST->as.unary.op);
            printf(")\n");
            print_AST_recursive(AST->as.unary.operand, child_prefix, true);
            break;

        case AST_GROUPING:
            printf("GROUPING\n");
            print_AST_recursive(AST->as.grouping.expression, child_prefix, true);
            break;

        case AST_BINARY:
            printf("BINARY (");
            print_AST_operation(AST->as.binary.op);
            printf(")\n");
            print_AST_recursive(AST->as.binary.left, child_prefix, false);
            print_AST_recursive(AST->as.binary.right, child_prefix, true);
            break;

        default:
            printf("UNKNOWN AST\n");
            break;
    }
}

static void print_AST_operation(tll_token_type operation)
{
    switch (operation)
    {
        case TOKEN_MINUS:
            printf("-");
            break;
        case TOKEN_PLUS:
            printf("+");
            break;
        case TOKEN_STAR:
            printf("*");
            break;
        case TOKEN_SLASH:
            printf("/");
            break;
        default:
            printf("?");
            break;
    }
}

static tll_AST* alloc_node(void)
{
    tll_AST empty = EMPTY_NODE;
    current_arena = add_to_arena(current_arena, &empty, sizeof(tll_AST));
    return (tll_AST*) (current_arena->available - sizeof(tll_AST));
}

static void AST_advance(void)
{
    AST_parser.previous = AST_parser.current;
    AST_parser.current++;
}

inline static bool AST_check(tll_token_type type)
{
    return AST_parser.current->type == type;
}

static bool AST_match(tll_token_type type)
{
    bool res = false;

    if (AST_check(type))
    {
        AST_advance();
        res = true;
    }
    return res;
}

static tll_AST* AST_expression(void)
{
    return AST_term();
}

static tll_AST* AST_term(void)
{
    tll_AST* left = AST_factor();

    while (AST_check(TOKEN_PLUS) || AST_check(TOKEN_MINUS))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* right = AST_factor();

        tll_AST* node = alloc_node();
        node->type = AST_BINARY;
        node->line = line;

        node->as.binary.op = op;
        node->as.binary.left = left;
        node->as.binary.right = right;

        left = node;
    }
    return left;
}

static tll_AST* AST_factor(void)
{
    tll_AST* left = AST_unary();

    while (AST_check(TOKEN_STAR) || AST_check(TOKEN_SLASH))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* right = AST_unary();

        tll_AST* node = alloc_node();
        node->type = AST_BINARY;
        node->line = line;

        node->as.binary.op = op;
        node->as.binary.left = left;
        node->as.binary.right = right;

        left = node;
    }
    return left;
}

static tll_AST* AST_unary(void)
{
    if (AST_check(TOKEN_MINUS))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* operand = AST_unary();

        tll_AST* node = alloc_node();
        node->type = AST_UNARY;
        node->line = line;

        node->as.unary.op = op;
        node->as.unary.operand = operand;

        return node;
    }
    return AST_primary();
}

static tll_AST* AST_primary(void)
{
    // Number literals
    if (AST_match(TOKEN_FLOAT_NUM))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;
        node->as.literal.value = FLOAT_VAL(strtod(AST_parser.previous->start, NULL));
        return node;
    }
    else if (AST_match(TOKEN_INT_NUM))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;
        node->as.literal.value = INT_VAL(atoi(AST_parser.previous->start));
        return node;
    }

    // Grouping
    if (AST_match(TOKEN_LEFT_PAREN))
    {
        int line = AST_parser.previous->line;
        tll_AST* expr = AST_expression();

        if (!AST_match(TOKEN_RIGHT_PAREN))
        {
            fprintf(stderr, "Expected ')' after expression.\n");
            return NULL;
        }
        tll_AST* node = alloc_node();
        node->type = AST_GROUPING;
        node->line = line;
        node->as.grouping.expression = expr;

        return node;
    }
    fprintf(stderr, "Expected expression.\n");
    return NULL;
}

