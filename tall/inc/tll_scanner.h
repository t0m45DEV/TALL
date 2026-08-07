#ifndef _TLL_SCANNER_H
#define _TLL_SCANNER_H

typedef enum {
    // Single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS,
    TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH,
    TOKEN_STAR, TOKEN_COLON,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS,
    TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL,
    TOKEN_RIGHT_ARROW,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_INT_NUM, TOKEN_FLOAT_NUM,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_CONST, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUNC, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, TOKEN_TRUE,
    TOKEN_VAR, TOKEN_WHILE, TOKEN_INT_TYPE, TOKEN_FLOAT_TYPE,
    TOKEN_STRING_TYPE, TOKEN_BOOL_TYPE,

    TOKEN_ERROR, TOKEN_EOF
} tll_token_type;

/**
 * The C representation for the lexical TALL tokens.
 */
typedef struct {
    tll_token_type type;
    const char* start;
    int length;
    int line;
} tll_token;

/**
 * Frees the memory used by the scanner.
 */
void free_scanner(void);

/**
 * Returns the given source_code in the form of a collection of TALL tokens, the last one is a TOKEN_EOF.
 */
tll_token* scan_source_code(const char* source_code);

#endif
