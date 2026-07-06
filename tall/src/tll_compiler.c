#include "tll_compiler.h"

#include "tll_AST.h"
#include "tll_code_chunk.h"
#include "tll_common.h"
#include "tll_scanner.h"
#include "tll_value.h"
#include "tll_object.h"

#include <stdint.h>
#include <stdio.h>

#ifdef DEBUG_PRINT_CODE
#include "tll_debug.h"
#endif

#define MAX_LOCAL_COUNT 256

typedef struct {
    const tll_string* name;
    int depth;
    int line;
} tll_local_var;

typedef struct {
    tll_local_var locals[MAX_LOCAL_COUNT];
    int local_count;
    int scope_depth;
} tll_compiler;

tll_compiler* current_compiler = NULL;

tll_code_chunk* compiling_code_chunk;

/**
 * Initialize the given compiler and sets it as the current one to be used as context.
 */
static void init_compiler(tll_compiler* compiler);

/**
 * Prints out the given message as a compiler error on the given line.
 */
static inline void compiler_error(const char* message, int line);

/**
 * Returns the code chunk now being compiled.
 */
static inline tll_code_chunk* current_code_chunk(void);

/**
 * Saves to the current code chunk the given byte, and saves it the given line number.
 */
static inline void emit_byte(uint8_t byte, int line);

/**
 * Emits the given short as two bytes on the current code chunk.
 */
static inline void emit_short(uint16_t byte, int line);

/**
 * Saves at the current code chunk an OP_RETURN.
 */
static inline void emit_return(int line);

/**
 * Saves two bytes, in the order they are given into the current code chunk.
 */
static inline void emit_bytes(uint8_t byte_1, uint8_t byte_2, int line);

/**
 * Returns the constants pool index for a given value.
 */
static uint16_t make_constant(tll_value value, int line);

/**
 * Saves a OP_CONSTANT with the constants pool index for the given value and saves it the given line info.
 */
static void emit_constant(tll_value value, int line);

/**
 * Returns the index of the given variable name that locales the local variable inside the given compiler context.
 *
 * Returns -1 otherwise.
 */
static int8_t resolve_local(const tll_compiler* compiler, const tll_string* var_name);

/**
 * Emits the bytes necessary for creating a global variable.
 */
static inline void define_global_variable(uint16_t global, int line);

/**
 * Emits the bytes necessary for creating a local variable.
 */
static inline void define_local_variable(const tll_string* var_name, int line);

/**
 * Emits the bytes necessary for assigning a local or a global variable.
 */
static void variable_assignment(const tll_string* var_name, int line);

/**
 * Emits the bytes necessary for reading a global or a local variable.
 */
static void named_variable(const tll_string* var_name, int line);

/**
 * Starts a new scope for local variables and code.
 */
static void begin_scope(void);

/**
 * Ends the current scope, poping all local variables.
 */
static void end_scope(void);

/**
 * Travels recursively through the given AST and saves bytecode to the current code chunk (see current_code_chunk).
 */
static void compile_AST_node(tll_AST* node);

static void init_compiler(tll_compiler* compiler)
{
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current_compiler = compiler;
}

static inline void compiler_error(const char* message, int line)
{
    fprintf(stderr, "[Line %i] Compiler error: %s\n", line, message);
}

bool compile_code(const char* source_code, tll_code_chunk* code_chunk)
{
    tll_compiler compiler;
    init_compiler(&compiler);

    compiling_code_chunk = code_chunk;
    tll_AST* AST = create_AST(scan_source_code(source_code));

    #ifdef DEBUG_PRINT_CODE
        print_AST(AST, "AST");
    #endif

    bool parsing_error = has_error(AST);

    if (!parsing_error)
    {
        compile_AST_node(AST);

        #ifdef DEBUG_PRINT_CODE
            disassemble_code_chunk(code_chunk, "BYTECODE");
        #endif
    }
    else
    {
        const tll_AST* error_node = get_error(AST);
        fprintf(stderr, "[Line %i] Parser error: %s\n", error_node->line, error_node->as.error.message);
    }
    end_AST(AST);
    free_scanner();

    // TODO: Error checking
    return !parsing_error;
}

static inline tll_code_chunk* current_code_chunk(void)
{
    return compiling_code_chunk;
}

static inline void emit_byte(uint8_t byte, int line)
{
    write_code_chunk(current_code_chunk(), byte, line);
}

static inline void emit_short(uint16_t byte, int line)
{
    emit_bytes((uint8_t) (byte >> 8), (uint8_t) (byte & 0xFF), line);
}

static inline void emit_return(int line)
{
    emit_byte(OP_RETURN, line);
}

static inline void emit_bytes(uint8_t byte_1, uint8_t byte_2, int line)
{
    emit_byte(byte_1, line);
    emit_byte(byte_2, line);
}

static uint16_t make_constant(tll_value value, int line)
{
    int constant = add_constant(current_code_chunk(), value);
    if (constant > UINT16_MAX)
    {
        compiler_error("Too many constants in one code chunk.", line);
        return 0;
    }
    return (uint16_t) constant;
}

static void emit_constant(tll_value value, int line)
{
    if (IS_NUMBER(value) || IS_STRING(value))
    {
        emit_byte(OP_CONSTANT, line);
        uint16_t const_index = make_constant(value, line);

        emit_short(const_index, line);
    }
    else
    {
        if (IS_NULL(value))
        {
            emit_byte(OP_NULL, line);
        }
        else if (AS_C_BOOL(value))
        {
            emit_byte(OP_TRUE, line);
        }
        else
        {
            emit_byte(OP_FALSE, line);
        }
    }
}

static int8_t resolve_local(const tll_compiler* compiler, const tll_string* var_name)
{
    for (int8_t i = compiler->local_count - 1; i >= 0; i--)
    {
        const tll_local_var* local = &compiler->locals[i];

        if (are_equals(AS_TLL_OBJ(local->name), AS_TLL_OBJ(var_name)))
        {
            return i;
        }
    }
    return -1;
}

static inline void define_global_variable(uint16_t global, int line)
{
    emit_byte(OP_DEFINE_GLOBAL, line);
    emit_short(global, line);
}

static inline void define_local_variable(const tll_string* var_name, int line)
{
    for (int i = current_compiler->local_count - 1; i >= 0; i--)
    {
        tll_local_var* local = &current_compiler->locals[i];

        if (local->depth != -1 && local->depth < current_compiler->scope_depth)
        {
            break;
        }
        if (are_equals(AS_TLL_OBJ(local->name), AS_TLL_OBJ(var_name)))
        {
            char message[100];
            sprintf(message, "Local variable '%s' already defined on line %i.", var_name->chars, local->line);
            compiler_error(message, line);
            return;
        }
    }
    if (current_compiler->local_count == MAX_LOCAL_COUNT)
    {
        compiler_error("Too many local variables in a single code block.", line);
        return;
    }
    tll_local_var* local = &current_compiler->locals[current_compiler->local_count];
    current_compiler->local_count++;
    local->name = var_name;
    local->line = line;
    local->depth = current_compiler->scope_depth;
}

static void variable_assignment(const tll_string* var_name, int line)
{
    int8_t arguments = resolve_local(current_compiler, var_name);

    if (arguments != (int8_t) -1)
    {
        emit_bytes(OP_SET_LOCAL, arguments, line);
    }
    else
    {
        emit_byte(OP_SET_GLOBAL, line);
        emit_short(make_constant(AS_TLL_OBJ(var_name), line), line);
    }
}

static void named_variable(const tll_string* var_name, int line)
{
    int8_t arguments = resolve_local(current_compiler, var_name);

    if (arguments != (int8_t) -1)
    {
        emit_bytes(OP_GET_LOCAL, arguments, line);
    }
    else
    {
        emit_byte(OP_GET_GLOBAL, line);
        emit_short(make_constant(AS_TLL_OBJ(var_name), line), line);
    }
}

static void begin_scope(void)
{
    current_compiler->scope_depth++;
}

static void end_scope(void)
{
    current_compiler->scope_depth--;

    while (current_compiler->local_count > 0 && current_compiler->locals[current_compiler->local_count - 1].depth > current_compiler->scope_depth)
    {
        emit_byte(OP_POP, current_compiler->locals[current_compiler->local_count - 1].line);
        current_compiler->local_count--;
    }
}

static void compile_AST_node(tll_AST* node)
{
    switch (node->type)
    {
        case AST_ERROR:
            break;

        case AST_LITERAL:
            emit_constant(node->as.literal.value, node->line);
            break;

        case AST_UNARY:
            compile_AST_node(node->as.unary.operand);

            switch (node->as.unary.op)
            {
                case TOKEN_BANG:
                    emit_byte(OP_NOT, node->line);
                    break;
                case TOKEN_MINUS:
                    emit_byte(OP_NEGATE, node->line);
                    break;
                default:
                    break; // Unreachable.
            }
            break;

        case AST_GROUPING:
            compile_AST_node(node->as.grouping.expression);
            break;

        case AST_BINARY:
            compile_AST_node(node->as.binary.left);
            compile_AST_node(node->as.binary.right);

            switch (node->as.binary.op)
            {
                case TOKEN_PLUS:
                    emit_byte(OP_ADD, node->line);
                    break;
                case TOKEN_MINUS:
                    emit_byte(OP_SUBSTRACT, node->line);
                    break;
                case TOKEN_STAR:
                    emit_byte(OP_MULTIPLY, node->line);
                    break;
                case TOKEN_SLASH:
                    emit_byte(OP_DIVIDE, node->line);
                    break;
                case TOKEN_EQUAL_EQUAL:
                    emit_byte(OP_EQUAL, node->line);
                    break;
                case TOKEN_BANG_EQUAL:
                    emit_byte(OP_NOT_EQUAL, node->line);
                    break;
                case TOKEN_GREATER:
                    emit_byte(OP_GREATER, node->line);
                    break;
                case TOKEN_GREATER_EQUAL:
                    emit_byte(OP_GREATER_EQUAL, node->line);
                    break;
                case TOKEN_LESS:
                    emit_byte(OP_LESS, node->line);
                    break;
                case TOKEN_LESS_EQUAL:
                    emit_byte(OP_LESS_EQUAL, node->line);
                    break;
            }
            break;

        case AST_PROGRAM:
            for (int i = 0; i < node->as.program.count; i++)
            {
                compile_AST_node(node->as.program.statements[i]);
            }
            break;

        case AST_EXPR_STATEMENT:
            compile_AST_node(node->as.expression_statement.expression);
            emit_byte(OP_POP, node->line);
            break;

        case AST_VAR_DECLARATION:
            compile_AST_node(node->as.var_declaration.expression);

            if (current_compiler->scope_depth == 0)
            {
                define_global_variable(make_constant(AS_TLL_OBJ(node->as.var_declaration.name), node->line), node->line);
            }
            else
            {
                define_local_variable(node->as.var_declaration.name, node->line);
            }
            break;

        case AST_VAR_ASSIGNMENT:
            compile_AST_node(node->as.var_assigment.expression);
            variable_assignment(node->as.var_assigment.name, node->line);
            break;

        case AST_VAR_NAME:
            named_variable(node->as.var_name.name, node->line);
            break;

        case AST_RETURN:
            if (node->as.return_statement.expression != NULL)
            {
                compile_AST_node(node->as.return_statement.expression);
            }
            else
            {
                emit_byte(OP_NULL, node->line);
            }
            emit_return(node->line);
            break;

        case AST_BLOCK:
            begin_scope();
            for (int i = 0; i < node->as.block_assignment.count; i++)
            {
                compile_AST_node(node->as.block_assignment.declarations[i]);
            }
            end_scope();
            break;

        default:
            return; // Unreachable.
    }
}

