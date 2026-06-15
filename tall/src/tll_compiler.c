#include "tll_compiler.h"

#include "tll_AST.h"
#include "tll_code_chunk.h"
#include "tll_common.h"
#include "tll_scanner.h"
#include "tll_value.h"

#ifdef DEBUG_PRINT_CODE
#include "tll_debug.h"
#endif

#include <stdio.h>

tll_code_chunk* compiling_code_chunk;

/**
 * Returns the code chunk now being compiled.
 */
static tll_code_chunk* current_code_chunk(void)
{
    return compiling_code_chunk;
}

/**
 * Saves to the current code chunk the given byte, and saves it the given line number.
 */
static void emit_byte(uint8_t byte, int line)
{
    write_code_chunk(current_code_chunk(), byte, line);
}

/**
 * Saves at the current code chunk an OP_RETURN.
 */
static void emit_return(int line)
{
    emit_byte(OP_RETURN, line);
}

/**
 * Saves two bytes, in the order they are given into the current code chunk.
 */
static void emit_bytes(uint8_t byte_1, uint8_t byte_2, int line)
{
    emit_byte(byte_1, line);
    emit_byte(byte_2, line);
}

/**
 * Returns the constants pool index for a given value.
 */
static uint16_t make_constant(tll_value value)
{
    int constant = add_constant(current_code_chunk(), value);
    if (constant > UINT16_MAX)
    {
        printf("[FATAL ERROR] Too many constant in one code chunk.\n");
        return 0;
    }
    return (uint16_t) constant;
}

/**
 * Saves a OP_CONSTANT with the constants pool index for the given value and saves it the given line info.
 */
static void emit_constant(tll_value value, int line)
{
    if (IS_NUMBER(value))
    {
        emit_byte(OP_CONSTANT, line);
        uint16_t const_index = make_constant(value);

        emit_bytes((uint8_t) (const_index >> 8), (uint8_t) (const_index & 0xFF), line);
    }
    else
    {
        if (IS_NULL(value))
        {
            emit_byte(OP_NULL, line);
        }
        else if (AS_BOOL(value))
        {
            emit_byte(OP_TRUE, line);
        }
        else
        {
            emit_byte(OP_FALSE, line);
        }
    }
}

/**
 * Travels recursively through the given AST and saves bytecode to the current code chunk (see current_code_chunk).
 */
static void compile_AST_node(tll_AST* node)
{
    switch (node->type)
    {
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

        default:
            return; // Unreachable.
    }
}

bool compile_code(const char* source_code, tll_code_chunk* code_chunk)
{
    compiling_code_chunk = code_chunk;
    tll_AST* AST = create_AST(scan_source_code(source_code));

    #ifdef DEBUG_PRINT_CODE
        print_AST(AST, "AST");
    #endif

    compile_AST_node(AST);
    emit_return(2);

    end_AST();
    free_scanner();

    #ifdef DEBUG_PRINT_CODE
        disassemble_code_chunk(code_chunk, "code");
    #endif

    // TODO: Error checking
    return true;
}

