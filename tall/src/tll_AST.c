#include "tll_AST.h"

#include "tll_arena.h"
#include "tll_memory.h"
#include "tll_scanner.h"
#include "tll_value.h"
#include "tll_object.h"

#include <stdatomic.h>
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
 * Free all the memory being used by the given tree.
 */
static void free_AST_recursive(tll_AST* tree);

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
 * Parse the current tokens as a while loop.
 */
static tll_AST* AST_while_loop(void);

/**
 * Parse the current tokens as a for loop.
 */
static tll_AST* AST_for_loop(void);

/**
 * Parse the current tokens as an if statement.
 */
static tll_AST* AST_if_statement(void);

/**
 * Parse the current tokens as a block of code between brackets.
 */
static tll_AST* AST_block(void);

/**
 * Parse the current tokens as a variable declaration.
 */
static tll_AST* AST_variable_declaration(bool is_const);

/**
 * Parse the current tokens as some variable assignment ('i = v', 'i++', 'i += v', etc), and checks for an ending semicolon if the given boolean is true.
 */
static tll_AST* AST_variable_assignment(bool check_semicolon);

/**
 * Parse the current tokens an specific variable assignment ('i = v'), and checks for an ending semicolon if the given boolean is true.
 */
static tll_AST* AST_specific_variable_assignment(bool check_semicolon);

/**
 * Parse the current tokens as a simple addition or substraction assignment (like 'i += v'), and checks for an ending semicolon if the given boolean is true.
 */
static tll_AST* AST_variable_simple_assignment(bool check_semicolon);

/**
 * Parse the current tokens as a quick modification for a defined variable (like 'i++' or 'i--'), and checks for an ending semicolon if the given boolean is true.
 */
static tll_AST* AST_variable_quick_modification(bool check_semicolon);

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
 * Parse the current tokens as a logical proposition.
 */
static tll_AST* AST_proposition(void);

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
    free_AST_recursive(tree);
    free_arena(first_arena);
    first_arena = NULL;
    current_arena = NULL;
}

static void free_AST_recursive(tll_AST* tree)
{
    switch (tree->type)
    {
        case AST_ERROR:
            return;

        case AST_LITERAL:
            return;

        case AST_UNARY:
            free_AST_recursive(tree->as.unary.operand);
            break;

        case AST_BINARY:
            free_AST_recursive(tree->as.binary.left);
            free_AST_recursive(tree->as.binary.right);
            break;

        case AST_GROUPING:
            free_AST_recursive(tree->as.grouping.expression);
            break;

        case AST_PROGRAM:
            for (int i = 0; i < tree->as.program.count; i++)
            {
                free_AST_recursive(tree->as.program.statements[i]);
            }
            FREE_ARRAY(tll_AST*, tree->as.program.statements, tree->as.program.count);
            break;

        case AST_EXPR_STATEMENT:
            free_AST_recursive(tree->as.expression_statement.expression);
            break;

        case AST_CONST_DECLARATION:
            free_AST_recursive(tree->as.const_declaration.expression);
            break;

        case AST_VAR_DECLARATION:
            free_AST_recursive(tree->as.var_declaration.expression);
            break;

        case AST_VAR_ASSIGNMENT:
            free_AST_recursive(tree->as.var_assigment.expression);
            break;

        case AST_VAR_NAME:
            break;

        case AST_RETURN:
            if (tree->as.return_statement.expression != NULL)
            {
                free_AST_recursive(tree->as.return_statement.expression);
            }
            break;

        case AST_BLOCK:
            for (int i = 0; i < tree->as.block_assignment.count; i++)
            {
                free_AST_recursive(tree->as.block_assignment.declarations[i]);
            }
            FREE_ARRAY(tll_AST*, tree->as.block_assignment.declarations, tree->as.block_assignment.count);
            break;

        case AST_IF:
            free_AST_recursive(tree->as.if_statement.condition);
            free_AST_recursive(tree->as.if_statement.code_block);

            if (tree->as.if_statement.else_block != NULL)
            {
                free_AST_recursive(tree->as.if_statement.else_block);
            }
            break;

        case AST_WHILE:
            free_AST_recursive(tree->as.while_loop.condition);
            free_AST_recursive(tree->as.while_loop.code_block);
            break;

        case AST_FOR:
            free_AST_recursive(tree->as.for_loop.initializer);
            free_AST_recursive(tree->as.for_loop.condition);
            free_AST_recursive(tree->as.for_loop.increment);
            free_AST_recursive(tree->as.for_loop.code_block);
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
    if (AST_match(TOKEN_FOR))
    {
        return AST_for_loop();
    }
    else if (AST_match(TOKEN_WHILE))
    {
        return AST_while_loop();
    }
    else if (AST_match(TOKEN_IF))
    {
        return AST_if_statement();
    }
    else if (AST_match(TOKEN_VAR))
    {
        return AST_variable_declaration(false);
    }
    else if (AST_match(TOKEN_CONST))
    {
        return AST_variable_declaration(true);
    }
    else if (AST_match(TOKEN_LEFT_BRACE))
    {
        return AST_block();
    }
    else if (AST_match(TOKEN_RETURN))
    {
        return AST_return_statement();
    }
    else if (AST_match(TOKEN_IDENTIFIER))
    {
        return AST_variable_assignment(true);
    }
    return AST_expression_statement();
}

static tll_AST* AST_while_loop(void)
{
    int line = AST_parser.previous->line;

    if (!AST_match(TOKEN_LEFT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected '(' after 'while'.");
    }
    tll_AST* condition = AST_expression();

    if (!AST_match(TOKEN_RIGHT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected ')' after 'while' condition. ");
    }
    if (!AST_match(TOKEN_LEFT_BRACE))
    {
        return AST_error(AST_parser.current->line, "Expected '{' at the start of the 'while' body.");
    }
    tll_AST* statements = AST_block();

    tll_AST* node = alloc_node();

    node->type = AST_WHILE;
    node->line = line;

    node->as.while_loop.condition = condition;
    node->as.while_loop.code_block = statements;

    return node;
}

static tll_AST* AST_for_loop(void)
{
    int line = AST_parser.previous->line;

    if (!AST_match(TOKEN_LEFT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected '(' after 'while'.");
    }
    tll_AST* initializer = NULL;

    if (AST_match(TOKEN_VAR))
    {
        initializer = AST_variable_declaration(false);
    }
    else if (AST_match(TOKEN_IDENTIFIER))
    {
        initializer = AST_variable_assignment(true);
    }
    else
    {
        initializer = AST_error(AST_parser.current->line, "Initializer for a loop must be a variable declaration or assignment.");
    }
    tll_AST* condition = AST_expression();

    if (!AST_match(TOKEN_SEMICOLON))
    {
        return AST_error(AST_parser.current->line, "Condition for a loop must be separated with ';'.");
    }
    tll_AST* increment = NULL;

    if (AST_match(TOKEN_IDENTIFIER))
    {
        increment = AST_variable_assignment(false);
    }
    else
    {
        increment = AST_error(AST_parser.current->line, "Increment step for a loop must be a variable assignment.");
    }

    if (!AST_match(TOKEN_RIGHT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected ')' after 'while' condition. ");
    }
    if (!AST_match(TOKEN_LEFT_BRACE))
    {
        return AST_error(AST_parser.current->line, "Expected '{' at the start of the 'while' body.");
    }
    tll_AST* statements = AST_block();

    tll_AST* node = alloc_node();

    node->type = AST_FOR;
    node->line = line;

    node->as.for_loop.initializer = initializer;
    node->as.for_loop.condition = condition;
    node->as.for_loop.increment = increment;
    node->as.for_loop.code_block = statements;

    return node;
}

static tll_AST* AST_if_statement(void)
{
    int line = AST_parser.current->line;

    if (!AST_match(TOKEN_LEFT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected '(' after 'if'.");
    }
    tll_AST* condition = AST_expression();

    if (!AST_match(TOKEN_RIGHT_PAREN))
    {
        return AST_error(AST_parser.current->line, "Expected ')' after 'if' condition.");
    }

    if (!AST_match(TOKEN_LEFT_BRACE))
    {
        return AST_error(AST_parser.current->line, "Expected '{' at the start of the 'if' body.");
    }
    tll_AST* statement = AST_block();

    tll_AST* else_block = NULL;

    if (AST_match(TOKEN_ELSE))
    {
        if (AST_match(TOKEN_IF))
        {
            else_block = AST_if_statement();
        }
        else if (AST_match(TOKEN_LEFT_BRACE))
        {
            else_block = AST_block();
        }
        else
        {
            else_block = AST_error(AST_parser.current->line, "Expected '{' or 'if' after 'else'.");
        }
    }
    tll_AST* node = alloc_node();

    node->type = AST_IF;
    node->line = line;

    node->as.if_statement.condition = condition;
    node->as.if_statement.code_block = statement;
    node->as.if_statement.else_block = else_block;

    return node;
}

static tll_AST* AST_block(void)
{
    tll_AST** statements = NULL;
    int count = 0;
    int capacity = 0;
    int line = AST_parser.current->line;

    while (!AST_check(TOKEN_EOF) && !AST_check(TOKEN_RIGHT_BRACE))
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
    if (!AST_match(TOKEN_RIGHT_BRACE))
    {
        return AST_error(line, "Expected '}' after block.");
    }
    tll_AST* node = alloc_node();
    node->type = AST_BLOCK;
    node->line = line;

    node->as.block_assignment.declarations = statements;
    node->as.block_assignment.count = count;

    return node;
}

static tll_AST* AST_variable_declaration(bool is_const)
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

    if (is_const)
    {
        node->type = AST_CONST_DECLARATION;
    }
    else
    {
        node->type = AST_VAR_DECLARATION;
    }
    node->line = line;
    node->as.var_declaration.name = var_name;
    node->as.var_declaration.type = var_type;
    node->as.var_declaration.expression = expr;

    return node;
}

static tll_AST* AST_variable_assignment(bool check_semicolon)
{
    if (AST_check(TOKEN_PLUS_PLUS) || AST_check(TOKEN_MINUS_MINUS))
    {
        return AST_variable_quick_modification(check_semicolon);
    }
    else if (AST_check(TOKEN_PLUS_EQUAL) || AST_check(TOKEN_MINUS_EQUAL))
    {
        return AST_variable_simple_assignment(check_semicolon);
    }
    else if (AST_check(TOKEN_EQUAL))
    {
        return AST_specific_variable_assignment(check_semicolon);
    }
    return AST_expression_statement();
}

static tll_AST* AST_specific_variable_assignment(bool check_semicolon)
{
    int line = AST_parser.previous->line;
    tll_string* var_name = copy_string(AST_parser.previous->start, AST_parser.previous->length);

    if (!AST_match(TOKEN_EQUAL))
    {
        return AST_error(AST_parser.current->line, "Expected '=' after variable assignment.");
    }
    tll_AST* expr = AST_expression();

    if (check_semicolon)
    {
        if (!AST_match(TOKEN_SEMICOLON))
        {
            return AST_error(AST_parser.current->line, "Variable assignment must end with ';'.");
        }
    }
    tll_AST* node = alloc_node();

    node->type = AST_VAR_ASSIGNMENT;
    node->line = line;
    node->as.var_assigment.name = var_name;
    node->as.var_assigment.expression = expr;

    return node;
}

static tll_AST* AST_variable_simple_assignment(bool check_semicolon)
{
    int line = AST_parser.previous->line;
    tll_string* var_name = copy_string(AST_parser.previous->start, AST_parser.previous->length);
    tll_token_type op;

    if (AST_match(TOKEN_PLUS_EQUAL))
    {
        op = TOKEN_PLUS;
    }
    else if (AST_match(TOKEN_MINUS_EQUAL))
    {
        op = TOKEN_MINUS;
    }
    else
    {
        return AST_error(AST_parser.current->line, "Invalid variable assignment.");
    }
    tll_AST* expr = AST_expression();

    if (check_semicolon)
    {
        if (!AST_match(TOKEN_SEMICOLON))
        {
            return AST_error(AST_parser.current->line, "Expected ';' after variable assignment.");
        }
    }
    tll_AST* var_node = alloc_node();

    var_node->line = line;
    var_node->type = AST_VAR_NAME;
    var_node->as.var_name.name = var_name;

    tll_AST* binary_node = alloc_node();

    binary_node->line = line;
    binary_node->type = AST_BINARY;
    binary_node->as.binary.op = op;
    binary_node->as.binary.left = var_node;
    binary_node->as.binary.right = expr;

    tll_AST* assignment_node = alloc_node();

    assignment_node->line = line;
    assignment_node->type = AST_VAR_ASSIGNMENT;
    assignment_node->as.var_assigment.name = var_name;
    assignment_node->as.var_assigment.expression = binary_node;

    return assignment_node;
}

static tll_AST* AST_variable_quick_modification(bool check_semicolon)
{
    int line = AST_parser.previous->line;
    tll_string* var_name = copy_string(AST_parser.previous->start, AST_parser.previous->length);
    tll_token_type op;

    if (AST_match(TOKEN_PLUS_PLUS))
    {
        op = TOKEN_PLUS;
    }
    else if (AST_match(TOKEN_MINUS_MINUS))
    {
        op = TOKEN_MINUS;
    }
    else
    {
        return AST_error(AST_parser.current->line, "Invalid variable assignment.");
    }

    if (check_semicolon)
    {
        if (!AST_match(TOKEN_SEMICOLON))
        {
            return AST_error(AST_parser.current->line, "Expected ';' after variable assignment.");
        }
    }
    tll_AST* number1_node = alloc_node();

    number1_node->line = line;
    number1_node->type = AST_LITERAL;
    number1_node->as.literal.value = AS_TLL_INT(1);

    tll_AST* var_node = alloc_node();

    var_node->line = line;
    var_node->type = AST_VAR_NAME;
    var_node->as.var_name.name = var_name;

    tll_AST* binary_node = alloc_node();

    binary_node->line = line;
    binary_node->type = AST_BINARY;
    binary_node->as.binary.op = op;
    binary_node->as.binary.left = var_node;
    binary_node->as.binary.right = number1_node;

    tll_AST* assignment_node = alloc_node();

    assignment_node->line = line;
    assignment_node->type = AST_VAR_ASSIGNMENT;
    assignment_node->as.var_assigment.name = var_name;
    assignment_node->as.var_assigment.expression = binary_node;

    return assignment_node;
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
    return AST_proposition();
}

static tll_AST* AST_proposition(void)
{
    tll_AST* left = AST_equality();

    while (AST_check(TOKEN_AND) || AST_check(TOKEN_OR))
    {
        AST_advance();

        tll_token_type op = AST_parser.previous->type;
        int line = AST_parser.previous->line;
        tll_AST* right = AST_equality();

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
    // We continue until the first ';' or '}' to synchronize.
    while (!AST_check(TOKEN_SEMICOLON) && !AST_check(TOKEN_RIGHT_BRACE) && !AST_check(TOKEN_EOF))
    {
        AST_advance(); // Advance ignoring the next stuff.
    }
    // If TOKEN_EOF is not reach yet, we can continue the parsing.
    if (AST_check(TOKEN_SEMICOLON))
    {
        AST_match(TOKEN_SEMICOLON);
    }
    else if (AST_check(TOKEN_RIGHT_BRACE))
    {
        AST_match(TOKEN_RIGHT_BRACE);
    }
    tll_AST* node = alloc_node();

    node->type = AST_ERROR;
    node->line = line;
    node->as.error.message = message;

    return node;
}

