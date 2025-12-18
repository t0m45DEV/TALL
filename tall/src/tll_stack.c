#include "tll_stack.h"

#include "tll_log.h"

#define STACK_LIMIT 10000

// The stack of the interpreter, literally a big array of floats
float stack[STACK_LIMIT];

// First FREE position in the stack
int stack_head = 0;

int stack_push(float element)
{
    if (stack_head >= STACK_LIMIT)
    {
        log_error("Stack size exceeded!");
        return 1;
    }
    stack[stack_head] = element;
    stack_head++;
    return 0;
}

int stack_pop(float* element)
{
    if (stack_head == 0)
    {
        log_error("Stack empty!");
        return 1;
    }
    stack_head--;
    *element = stack[stack_head];
    return 0;
}

int stack_peek(float* element, int i)
{
    if (i < 0 || stack_head < i)
    {
        log_error("Index %i out of stack range!", i);
        return 1;
    }
    *element = stack[i];
    return 0;
}

int stack_size(void)
{
    return stack_head;
}

void stack_clear(void)
{
    stack_head = 0;
}

