#include "tll_AST.h"

#include "tll_arena.h"
#include "tll_memory.h"
#include "tll_scanner.h"
#include "tll_value.h"
#include "tll_object.h"

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
static inline bool AST_check(tll_token_type type);

/**
 * Returns true if the current token being parsed has the same type as the given one, and if that's the case advances the AST.
 */
static bool AST_match(tll_token_type type);

/**
 * Generates a TALL program AST from the given tokens collection, for parsing scripts.
 */
static tll_AST* AST_program(void);

/**
 * Parse the current tokens as an statement.
 */
static tll_AST* AST_statement(void);

/**
 * Parse the current tokens as a variable declaration.
 */
static tll_AST* AST_variable_declaration(void);

/**
 * Parse the current tokens as an statement of an expression (a function call or an expression discarding the value).
 */
static tll_AST* AST_expression_statement(void);

/**
 * Parse the current token as a return statement.
 */
static tll_AST* AST_return_statement(void);

/**
 * Parse the current tokens as an expression.
 */
static tll_AST* AST_expression(void);

/**
 * Parse the current token as a equality check (==, !=)
 */
static tll_AST* AST_equality(void);

/**
 * Parse the current token as a logical comparison (<=, >=)
 */
static tll_AST* AST_comparison(void);

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

/**
 * Returns an AST node with type error, using the given line and message as info.
 */
static tll_AST* AST_error(int line, const char* message);

tll_AST* create_AST(tll_token* tokens)
{
    first_arena = create_arena();
    current_arena = first_arena;

    AST_parser.current = tokens;
    AST_parser.previous = tokens;

    return AST_program();
}

void end_AST(tll_AST* tree)
{
    if (tree->type == AST_PROGRAM)
    {
        FREE_ARRAY(tll_AST*, tree->as.program.statements, tree->as.program.count);
    }

    free_arena(first_arena);
    first_arena = NULL;
    current_arena = NULL;
}

void print_AST(const tll_AST* AST, const char* name)
{
    printf("%s\n", name);
    print_AST_recursive(AST, "", true);
}

bool has_error(const tll_AST* AST)
{
    bool res = false;

    switch (AST->type)
    {
        case AST_ERROR:
            res = true;
            break;

        case AST_LITERAL:
            break;

        case AST_UNARY:
            res = has_error(AST->as.unary.operand);
            break;

        case AST_GROUPING:
            res = has_error(AST->as.grouping.expression);
            break;

        case AST_BINARY:
            res = has_error(AST->as.binary.left) || has_error(AST->as.binary.right);
            break;

        case AST_PROGRAM:

            for (int i = 0; i < AST->as.program.count; i++)
            {
                res = has_error(AST->as.program.statements[i]);

                if (res)
                {
                    break;
                }
            }
            break;

        case AST_EXPR_STATEMENT:
            res = has_error(AST->as.expression_statement.expression);
            break;

        case AST_VAR_DECLARATION:
            res = has_error(AST->as.var_declaration.expression);
            break;

        case AST_VAR_NAME:
            break;

        case AST_RETURN:
            if (AST->as.return_statement.expression != NULL)
            {
                res = has_error(AST->as.return_statement.expression);
            }
            break;
    }
    return res;
}

tll_AST* get_error(tll_AST* AST)
{
    tll_AST* temp;
    tll_AST* res = NULL;

    switch (AST->type)
    {
        case AST_ERROR:
            res = AST;
            break;

        case AST_LITERAL:
            break;

        case AST_UNARY:
            res = get_error(AST->as.unary.operand);
            break;

        case AST_GROUPING:
            res = get_error(AST->as.grouping.expression);
            break;

        case AST_BINARY:
            temp = get_error(AST->as.binary.left);

            if (temp == NULL)
            {
                res = get_error(AST->as.binary.right);
            }
            else
            {
                res = temp;
            }
            break;

        case AST_PROGRAM:

            for (int i = 0; i < AST->as.program.count; i++)
            {
                res = get_error(AST->as.program.statements[i]);

                if (res != NULL)
                {
                    break;
                }
            }
            break;

        case AST_EXPR_STATEMENT:
            res = get_error(AST->as.expression_statement.expression);
            break;

        case AST_VAR_DECLARATION:
            res = get_error(AST->as.var_declaration.expression);
            break;

        case AST_VAR_NAME:
            break;

        case AST_RETURN:
            if (AST->as.return_statement.expression != NULL)
            {
                res = get_error(AST->as.return_statement.expression);
            }
            break;
    }
    return res;
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
        case AST_ERROR:
            printf("ERROR: %s\n", AST->as.error.message);
            break;

        case AST_LITERAL:
            printf("LITERAL (");
            print_type(AST->as.literal.value);

            if (!IS_NULL(AST->as.literal.value))
            {
                printf(" ");
                print_value(AST->as.literal.value);
            }
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

        case AST_PROGRAM:
            printf("PROGRAM\n");
            for (int i = 0; i < AST->as.program.count - 1; i++)
            {
                print_AST_recursive(AST->as.program.statements[i], child_prefix, false);
            }
            if (AST->as.program.count > 0)
            {
                print_AST_recursive(AST->as.program.statements[AST->as.program.count - 1], child_prefix, true);
            }
            break;

        case AST_EXPR_STATEMENT:
            printf("EXPRESSION_STATEMENT\n");
            print_AST_recursive(AST->as.expression_statement.expression, child_prefix, true);
            break;

        case AST_VAR_DECLARATION:
            printf("VAR_DECLARATION (");
            printf("%.*s : ", AST->as.var_declaration.name->length, AST->as.var_declaration.name->chars);

            switch (AST->as.var_declaration.type)
            {
                case VAL_NULL:
                    printf("null");
                    break;

                case VAL_BOOL:
                    printf("bool");
                    break;

                case VAL_INT:
                    printf("int");
                    break;

                case VAL_FLOAT:
                    printf("float");
                    break;

                case VAL_OBJ: // TODO: Better reading type for objects.
                    printf("string");
                    break;
            }
            printf(")\n");
            print_AST_recursive(AST->as.var_declaration.expression, child_prefix, true);
            break;

        case AST_VAR_NAME:
            printf("VAR_NAME (");
            printf("%.*s", AST->as.var_name.name->length, AST->as.var_name.name->chars);
            printf(")\n");
            break;

        case AST_RETURN:
            printf("RETURN\n");
            if (AST->as.return_statement.expression != NULL)
            {
                print_AST_recursive(AST->as.return_statement.expression, child_prefix, true);
            }
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
        case TOKEN_BANG:
            printf("!");
            break;
        case TOKEN_EQUAL_EQUAL:
            printf("==");
            break;
        case TOKEN_BANG_EQUAL:
            printf("!=");
            break;
        case TOKEN_GREATER:
            printf(">");
            break;
        case TOKEN_GREATER_EQUAL:
            printf(">=");
            break;
        case TOKEN_LESS:
            printf("<");
            break;
        case TOKEN_LESS_EQUAL:
            printf("<=");
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

static inline bool AST_check(tll_token_type type)
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

static tll_AST* AST_program(void)
{
    tll_AST** statements = NULL;
    int count = 0;
    int capacity = 0;
    int line = AST_parser.current->line;

    while (!AST_check(TOKEN_EOF))
    {
        if (count + 1 > capacity)
        {
            int old_capacity = capacity;
            capacity = GROW_CAPACITY(capacity);
            statements = GROW_ARRAY(tll_AST*, statements, old_capacity, capacity);
        }
        statements[count] = AST_statement();
        count++;
    }
    tll_AST* node = alloc_node();
    node->type = AST_PROGRAM;
    node->line = line;

    node->as.program.statements = statements;
    node->as.program.count = count;

    return node;
}

static tll_AST* AST_statement(void)
{
    if (AST_match(TOKEN_VAR))
    {
        return AST_variable_declaration();
    }
    else if (AST_match(TOKEN_RETURN))
    {
        return AST_return_statement();
    }
    return AST_expression_statement();
}

static tll_AST* AST_variable_declaration(void)
{
    int line = AST_parser.current->line;

    if (!AST_match(TOKEN_IDENTIFIER))
    {
        return AST_error(line, "Invalid variable name.");
    }
    tll_string* var_name = copy_string(AST_parser.previous->start, AST_parser.previous->length);

    if (!AST_match(TOKEN_COLON))
    {
        return AST_error(line, "Expected ':' after variable name.");
    }
    tll_value_type var_type = VAL_NULL;

    if (AST_match(TOKEN_BOOL_TYPE))
    {
        var_type = VAL_BOOL;
    }
    else if (AST_match(TOKEN_INT_TYPE))
    {
        var_type = VAL_INT;
    }
    else if (AST_match(TOKEN_FLOAT_TYPE))
    {
        var_type = VAL_FLOAT;
    }
    else if (AST_match((TOKEN_STRING_TYPE)))
    {
        var_type = VAL_OBJ;
    }
    else
    {
        return AST_error(line, "Variable must have a valid type.");
    }

    if (!AST_match(TOKEN_EQUAL))
    {
        return AST_error(line, "Variables must be initialized with a value.");
    }
    tll_AST* expr = AST_expression();

    if (!AST_match(TOKEN_SEMICOLON))
    {
        return AST_error(line, "Variable declaration must end with ';'.");
    }
    tll_AST* node = alloc_node();

    node->type = AST_VAR_DECLARATION;
    node->line = line;
    node->as.var_declaration.name = var_name;
    node->as.var_declaration.type = var_type;
    node->as.var_declaration.expression = expr;

    return node;
}

static tll_AST* AST_expression_statement(void)
{
    int line = AST_parser.previous->line;

    tll_AST* expr = AST_expression();

    if (!AST_match(TOKEN_SEMICOLON))
    {
        return AST_error(line, "Expected ';' after expression statement.");
    }
    tll_AST* node = alloc_node();

    node->type = AST_EXPR_STATEMENT;
    node->line = line;
    node->as.expression_statement.expression = expr;

    return node;
}

static tll_AST* AST_return_statement(void)
{
    int line = AST_parser.previous->line;
    tll_AST* node = alloc_node();
    node->type = AST_RETURN;
    node->line = line;

    if (AST_check(TOKEN_SEMICOLON))
    {
        node->as.return_statement.expression = NULL;
        AST_match(TOKEN_SEMICOLON);
    }
    else
    {
        node->as.return_statement.expression = AST_expression();

        if (!AST_match(TOKEN_SEMICOLON))
        {
            return AST_error(line, "Expected ';' after return expression.");
        }
    }
    return node;
}

static tll_AST* AST_expression(void)
{
    return AST_equality();
}

static tll_AST* AST_equality(void)
{
    tll_AST* left = AST_comparison();

    while (AST_check(TOKEN_EQUAL_EQUAL) || AST_check(TOKEN_BANG_EQUAL))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* right = AST_comparison();

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

static tll_AST* AST_comparison(void)
{
    tll_AST* left = AST_term();

    while (AST_check(TOKEN_GREATER) || AST_check(TOKEN_GREATER_EQUAL) || AST_check(TOKEN_LESS) || AST_check(TOKEN_LESS_EQUAL))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* right = AST_term();

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
    if (AST_check(TOKEN_MINUS) || AST_check(TOKEN_BANG))
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
        node->as.literal.value = AS_TLL_FLOAT(strtod(AST_parser.previous->start, NULL));
        return node;
    }
    else if (AST_match(TOKEN_INT_NUM))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;
        node->as.literal.value = AS_TLL_INT(atoi(AST_parser.previous->start));
        return node;
    }

    if (AST_match(TOKEN_STRING))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;
        node->as.literal.value = AS_TLL_OBJ(copy_string(AST_parser.previous->start + 1, AST_parser.previous->length - 2));
        return node;
    }

    // Boolean
    if (AST_match(TOKEN_TRUE) || AST_match(TOKEN_FALSE))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;

        if (*AST_parser.previous->start == 't')
        {
            node->as.literal.value = AS_TLL_BOOL(true);
        }
        else
        {
            node->as.literal.value = AS_TLL_BOOL(false);
        }
        return node;
    }

    // Null
    if (AST_match(TOKEN_NIL))
    {
        tll_AST* node = alloc_node();
        node->type = AST_LITERAL;
        node->line = AST_parser.previous->line;
        node->as.literal.value = AS_TLL_NULL;
        return node;
    }

    // Identifier
    if (AST_match(TOKEN_IDENTIFIER))
    {
        tll_AST* node = alloc_node();
        node->type = AST_VAR_NAME;
        node->line = AST_parser.previous->line;
        node->as.var_name.name = copy_string(AST_parser.previous->start, AST_parser.previous->length);
        return node;
    }

    // Grouping
    if (AST_match(TOKEN_LEFT_PAREN))
    {
        int line = AST_parser.previous->line;
        tll_AST* expr = AST_expression();

        if (!AST_match(TOKEN_RIGHT_PAREN))
        {
            return AST_error(line, "Expected ')' after expression.");
        }
        tll_AST* node = alloc_node();

        node->type = AST_GROUPING;
        node->line = line;
        node->as.grouping.expression = expr;

        return node;
    }
    return AST_error(AST_parser.previous->line, "Expected expression.");
}

static tll_AST* AST_error(int line, const char* message)
{
    // We continue until the first ';' to synchronize.
    while (!AST_check(TOKEN_SEMICOLON) && !AST_check(TOKEN_EOF))
    {
        AST_advance(); // Advance ignoring the next stuff.
    }
    // If TOKEN_EOF is not reach yet, we can continue the parsing.
    AST_match(TOKEN_SEMICOLON);

    tll_AST* node = alloc_node();

    node->type = AST_ERROR;
    node->line = line;
    node->as.error.message = message;

    return node;
}

