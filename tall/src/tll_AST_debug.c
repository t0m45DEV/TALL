#include "tll_AST.h"

#include "tll_object.h"

#include <stdio.h>

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

void print_AST(const tll_AST* AST, const char* name)
{
    printf("=== %s ===\n", name);
    print_AST_recursive(AST, "", true);
}

bool has_error(const tll_AST* AST)
{
    return get_error(AST) != NULL;
}

const tll_AST* get_error(const tll_AST* AST)
{
    const tll_AST* temp;
    const tll_AST* res = NULL;

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

        case AST_VAR_ASSIGNMENT:
            res = get_error(AST->as.var_assigment.expression);
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
            print_string(*AST->as.var_declaration.name);

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

        case AST_VAR_ASSIGNMENT:
            printf("VAR_ASSIGNMENT (");
            print_string(*AST->as.var_assigment.name);
            printf(")\n");
            print_AST_recursive(AST->as.var_assigment.expression, child_prefix, true);
            break;

        case AST_VAR_NAME:
            printf("VAR_NAME (");
            print_string(*AST->as.var_name.name);
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

