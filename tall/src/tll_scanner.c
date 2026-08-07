#include "tll_scanner.h"

#include "tll_common.h"
#include "tll_memory.h"

#include <stdio.h>
#include <string.h>

/**
 * A lexemes scanner, saves the start of the current lexeme, the file line it is, and the current character being read.
 */
typedef struct {
    const char* start;
    const char* current;
    int line;
    struct {
        int count;
        int capacity;
        tll_token* list;
    } tokens;
} tll_scanner;

// The tracker for the current lexeme on the source code.
tll_scanner scanner;

/**
 * Initialize the scanner for the lexical analisis.
 */
static void init_scanner(const char* source_code);

/**
 * Returns the next token on the scanner.
 */
static tll_token scan_token(void);

/**
 * Returns true if the scanner is currently at the end of the file.
 */
static inline bool is_at_end(void);

/**
 * Returns true if the given character is a digit.
 */
static inline bool is_digit(char c);

/**
 * Returns true if the given character is a letter a '_'.
 */
static inline bool is_alpha(char c);

/**
 * Returns the character being read at the current lexeme, and advances the scanner to the next character.
 */
static inline char advance(void);

/**
 * Returns the character being read at the current lexeme.
 */
static inline char peek(void);

/**
 * Returns the next character of the one being read at the current lexeme, is a "look ahead" function.
 *
 * If the scanner is at the end of the source code, it returns '\0'.
 */
static char peek_next(void);

/**
 * Returns true if the given character is the same at the one being read by the scanner, then advances the scanner to the next character.
 */
static bool match(char expected);

/**
 * Advances the scanner until the next lexeme, omiting spaces, new lines, tabulations and comments.
 */
static void skip_white_space(void);

/**
 * Returns the type of the lexeme currently being seen by the scanner.
 */
static tll_token_type identifier_type(void);

/**
 * Compares the lexeme currently being seen by the scanner with the given rest string.
 *
 * If they are equal, returns the given TALL token type.
 * Returns TOKEN_IDENTIFIER otherwise.
 */
static tll_token_type check_keyword(int start, int length, const char* rest, tll_token_type type);

/**
 * Creates a TALL token with the given type.
 */
static tll_token make_token(tll_token_type type);

/**
 * Creates a TALL token with type TOKEN_ERROR and the given message.
 */
static tll_token error_token(const char* message);

/**
 * Returns the token of type TOKEN_NUMBER, it's value is the one being currently seen by the scanner, until it reaches another lexeme.
 *
 * If the value contains a '.' it is considered a float.
 */
static tll_token number(void);

/**
 * Returns the token of type TOKEN_STRING, it's text is the one being currently seen by the scanner, until it reaches the final '"'.
 *
 * If that '"' is not found, then it returns an token of type TOKEN_ERROR.
 */
static tll_token string(void);

/**
 * Returns the current lexeme in the form of a TALL token with type identifier or TALL keyword.
 */
static tll_token identifier(void);

void free_scanner(void)
{
    FREE_ARRAY(tll_token, scanner.tokens.list, scanner.tokens.capacity);
    init_scanner(NULL);
}

tll_token* scan_source_code(const char* source_code)
{
    init_scanner(source_code);

    tll_token current = scan_token();

    while (current.type != TOKEN_EOF)
    {
        if (scanner.tokens.count + 1 > scanner.tokens.capacity)
        {
            int old_capacity = scanner.tokens.capacity;
            scanner.tokens.capacity = GROW_CAPACITY(old_capacity);
            scanner.tokens.list = GROW_ARRAY(tll_token, scanner.tokens.list, old_capacity, scanner.tokens.capacity);
        }
        scanner.tokens.list[scanner.tokens.count] = current;
        scanner.tokens.list[scanner.tokens.count].line = scanner.line;
        scanner.tokens.count++;

        current = scan_token();
    }

    // We add the TOKEN_EOF at the end, that it should be current
    if (scanner.tokens.count + 1 > scanner.tokens.capacity)
    {
        scanner.tokens.list = GROW_ARRAY(tll_token, scanner.tokens.list, scanner.tokens.capacity, scanner.tokens.capacity + 1);
        scanner.tokens.capacity++;
    }
    scanner.tokens.list[scanner.tokens.count] = current;
    scanner.tokens.list[scanner.tokens.count].line = scanner.line;
    scanner.tokens.count++;

    return scanner.tokens.list;
}

static void init_scanner(const char* source_code)
{
    scanner.start = source_code;
    scanner.current = source_code;
    scanner.line = 1;
    scanner.tokens.count = 0;
    scanner.tokens.capacity = 0;
    scanner.tokens.list = NULL;
}

static tll_token scan_token(void)
{
    skip_white_space();
    scanner.start = scanner.current;

    if (is_at_end())
    {
        return make_token(TOKEN_EOF);
    }
    char c = advance();

    if (is_alpha(c))
    {
        return identifier();
    }
    if (is_digit(c))
    {
        return number();
    }

    switch (c)
    {
        case '(':
        {
            return make_token(TOKEN_LEFT_PAREN);
        }
        case ')':
        {
            return make_token(TOKEN_RIGHT_PAREN);
        }
        case '{':
        {
            return make_token(TOKEN_LEFT_BRACE);
        }
        case '}':
        {
            return make_token(TOKEN_RIGHT_BRACE);
        }
        case ':':
        {
            return make_token(TOKEN_COLON);
        }
        case ';':
        {
            return make_token(TOKEN_SEMICOLON);
        }
        case ',':
        {
            return make_token(TOKEN_COMMA);
        }
        case '.':
        {
            return make_token(TOKEN_DOT);
        }
        case '-':
        {
            if (match('-'))
            {
                return make_token(TOKEN_MINUS_MINUS);
            }
            else if (match('='))
            {
                return make_token(TOKEN_MINUS_EQUAL);
            }
            else if (match('>'))
            {
                return make_token(TOKEN_RIGHT_ARROW);
            }
            else
            {
                return make_token(TOKEN_MINUS);
            }
        }
        case '+':
        {
            if (match('+'))
            {
                return make_token(TOKEN_PLUS_PLUS);
            }
            else if (match('='))
            {
                return make_token(TOKEN_PLUS_EQUAL);
            }
            else
            {
                return make_token(TOKEN_PLUS);
            }
        }
        case '/':
        {
            return make_token(TOKEN_SLASH);
        }
        case '*':
        {
            return make_token(TOKEN_STAR);
        }
        case '!':
        {
            return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        }
        case '=':
        {
            return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        }
        case '<':
        {
            return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        }
        case '>':
        {
            return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        }
        case '"':
        {
            return string();
        }
    }
    return error_token("Unexpected character.");
}

static inline bool is_at_end(void)
{
    return (*scanner.current == '\0');
}

static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static inline bool is_alpha(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

static inline char advance(void)
{
    scanner.current++;
    return scanner.current[-1];
}

static inline char peek(void)
{
    return *scanner.current;
}

static char peek_next(void)
{
    if (is_at_end())
    {
        return '\0';
    }
    return scanner.current[1];
}

static bool match(char expected)
{
    if (is_at_end())
    {
        return false;
    }
    if (*scanner.current != expected)
    {
        return false;
    }
    scanner.current++;
    return true;
}

static void skip_white_space(void)
{
    while (true)
    {
        char c = peek();

        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
            {
                advance();
                break;
            }
            case '\n':
            {
                scanner.line++;
                advance();
                break;
            }
            case '/':
            {
                if (peek_next() == '/')
                {
                    // A comment goes until the end of the line
                    while (peek() != '\n' && !is_at_end())
                    {
                        advance();
                    }
                    if (peek() == '\n')
                    {
                        advance();
                        scanner.line++;
                    }
                    break;
                }
                else
                {
                    return;
                }
            }
            default:
            {
                return;
            }
        }
    }
}

static tll_token_type identifier_type(void)
{
    switch (scanner.start[0])
    {
        case 'a':
        {
            return check_keyword(1, 2, "nd", TOKEN_AND);
        }
        case 'b':
        {
            return check_keyword(1, 3, "ool", TOKEN_BOOL_TYPE);
        }
        case 'c':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'l':
                    {
                        return check_keyword(2, 3, "ass", TOKEN_CLASS);
                    }
                    case 'o':
                    {
                        return check_keyword(2, 3, "nst", TOKEN_CONST);
                    }
                }
            }
            break;
        }
        case 'e':
        {
            return check_keyword(1, 3, "lse", TOKEN_ELSE);
        }
        case 'i':
        {
            if (scanner.current - scanner.start > 1)
            {
                if (scanner.current - scanner.start == 2)
                {
                    if (scanner.start[1] == 'f')
                    {
                        return TOKEN_IF;
                    }
                    break;
                }
                return check_keyword(1, 2, "nt", TOKEN_INT_TYPE);
            }
            break;
        }
        case 'n':
        {
            return check_keyword(1, 3, "ull", TOKEN_NIL);
        }
        case 'o':
        {
            return check_keyword(1, 1, "r", TOKEN_OR);
        }
        case 'r':
        {
            return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        }
        case 's':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'u':
                    {
                        return check_keyword(2, 3, "per", TOKEN_SUPER);
                    }
                    case 't':
                    {
                        return check_keyword(2, 4, "ring", TOKEN_STRING_TYPE);
                    }
                }
            }
            break;
        }
        case 'v':
        {
            return check_keyword(1, 2, "ar", TOKEN_VAR);
        }
        case 'w':
        {
            return check_keyword(1, 4, "hile", TOKEN_WHILE);
        }
        case 'f':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'a':
                    {
                        return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    }
                    case 'l':
                    {
                        return check_keyword(2, 3, "oat", TOKEN_FLOAT_TYPE);
                    }
                    case 'o':
                    {
                        return check_keyword(2, 1, "r", TOKEN_FOR);
                    }
                    case 'u':
                    {
                        return check_keyword(2, 2, "nc", TOKEN_FUNC);
                    }
                }
            }
            break;
        }
        case 't':
        {
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'h':
                    {
                        return check_keyword(2, 2, "is", TOKEN_THIS);
                    }
                    case 'r':
                    {
                        return check_keyword(2, 2, "ue", TOKEN_TRUE);
                    }
                }
            }
            break;
        }
    }
    return TOKEN_IDENTIFIER;
}

static tll_token_type check_keyword(int start, int length, const char* rest, tll_token_type type)
{
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0)
    {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static tll_token make_token(tll_token_type type)
{
    tll_token token;

    token.type = type;
    token.start = scanner.start;
    token.length = (int) (scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static tll_token error_token(const char* message)
{
    tll_token token;

    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner.line;

    return token;
}

static tll_token number(void)
{
    tll_token_type type = TOKEN_INT_NUM;

    while (is_digit(peek()))
    {
        advance();
    }
    if (peek() == '.' && is_digit(peek_next()))
    {
        // Consume the '.'
        advance();
        type = TOKEN_FLOAT_NUM;

        while (is_digit(peek()))
        {
            advance();
        }
    }
    return make_token(type);
}

static tll_token string(void)
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n')
        {
            scanner.line++;
        }
        advance();
    }
    if (is_at_end()) return error_token("Unterminated string.");

    // The closing quote.
    advance();
    return make_token(TOKEN_STRING);
}

static tll_token identifier(void)
{
    while (is_alpha(peek()) || is_digit(peek()))
    {
        advance();
    }
    return make_token(identifier_type());
}

