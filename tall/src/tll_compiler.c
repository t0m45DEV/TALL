#include "tll_compiler.h"

#include "tll_common.h"
#include "tll_scanner.h"

#include <stdio.h>

void compile_code(const char *source_code)
{
    init_scanner(source_code);

    int line = -1;
    while (true)
    {
        tll_token token = scan_token();

        if (token.line != line)
        {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
        {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) break;
    }
}

