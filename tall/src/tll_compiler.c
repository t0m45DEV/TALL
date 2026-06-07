#include "tll_compiler.h"

#include "tll_code_chunk.h"
#include "tll_common.h"
#include "tll_scanner.h"
#include "tll_value.h"

#ifdef DEBUG_PRINT_CODE
#include "tll_debug.h"
#endif

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    tll_token current;
    tll_token previous;
    bool had_error;
    bool panic_mode;
} tll_parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} tll_precedence;

typedef void (*tll_parse_func)(void);

typedef struct {
    tll_parse_func prefix;
    tll_parse_func infix;
    tll_precedence precedence;
} tll_parse_rule;

static void number(void);
static void grouping(void);
static void unary(void);
static void binary(void);

tll_parse_rule parse_rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

tll_parser parser;

tll_code_chunk* compiling_code_chunk;

static tll_code_chunk* current_code_chunk(void)
{
    return compiling_code_chunk;
}

static void error_at(tll_token* token, const char* message)
{
    if (parser.panic_mode)
    {
        // Avoid error cascades.
        return;
    }
    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Do nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char* message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char* message)
{
    error_at(&parser.current, message);
}

static void emit_byte(uint8_t byte)
{
    write_code_chunk(current_code_chunk(), byte, parser.previous.line);
}

/**
 * Moves the parser to the next token, if an error is detected it repors it.
 */
static void advance_parser(void)
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scan_token();

        if (parser.current.type != TOKEN_ERROR)
        {
            break;
        }
        error_at_current(parser.current.start);
    }
}

/**
 * Checks if the current token being seen by the parser has the same type as the given one.
 *
 * If true advances the parser to the next token.
 * If false an error with the given message is reported.
 */
static void consume_token(tll_token_type type, const char* message)
{
    if (parser.current.type == type)
    {
        advance_parser();
    }
    else
    {
        error_at_current(message);
    }
}

static void emit_return(void)
{
    emit_byte(OP_RETURN);
}

static void end_compiler(void)
{
    emit_return();
    #ifdef DEBUG_PRINT_CODE
    if (!parser.had_error)
    {
        disassemble_code_chunk(current_code_chunk(), "code");
    }
    #endif
}

static void emit_bytes(uint8_t byte_1, uint8_t byte_2)
{
    emit_byte(byte_1);
    emit_byte(byte_2);
}

static uint8_t make_constant(tll_value value)
{
    int constant = add_constant(current_code_chunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constant in one code chunk.");
        return 0;
    }
    return (uint8_t) constant;
}

static void emit_constant(tll_value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void number(void)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(value);
}

static tll_parse_rule* get_parse_rule(tll_token_type type)
{
    return &parse_rules[type];
}

static void parse_precedence(tll_precedence precedence)
{
    advance_parser();

    tll_parse_func prefix_rule = get_parse_rule(parser.previous.type)->prefix;

    if (prefix_rule == NULL)
    {
        error("Expect expression.");
        return;
    }
    prefix_rule();

    while (precedence <= get_parse_rule(parser.current.type)->precedence)
    {
        advance_parser();
        tll_parse_func infix_rule = get_parse_rule(parser.previous.type)->infix;
        infix_rule();
    }
}

static void expression(void)
{
    parse_precedence(PREC_ASSIGNMENT);
}


static void grouping(void)
{
    expression();
    consume_token(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(void)
{
    tll_token_type operator_type = parser.previous.type;

    // Compile the operand.
    parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operator_type)
    {
        case TOKEN_MINUS:
            emit_byte(OP_NEGATE);
            break;

        default:
            return; // Unreachable.
    }
}

static void binary(void)
{
    tll_token_type operator_type = parser.previous.type;
    tll_parse_rule* rule = get_parse_rule(operator_type);

    parse_precedence((tll_precedence) (rule->precedence + 1));

    switch (operator_type)
    {
        case TOKEN_PLUS:
            emit_byte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emit_byte(OP_SUBSTRACT);
            break;
        case TOKEN_STAR:
            emit_byte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emit_byte(OP_DIVIDE);
            break;
        default:
            return; // Unreachable.

    }
}

bool compile_code(const char* source_code, tll_code_chunk* code_chunk)
{
    init_scanner(source_code);
    compiling_code_chunk = code_chunk;
    parser.had_error = false;
    parser.panic_mode = false;

    advance_parser();
    expression();
    consume_token(TOKEN_EOF, "Expect end of expression");

    end_compiler();
    return !parser.had_error;
}

