#include "tll_stack.h"

#include "tll_log.h"

#define STACK_LIMIT 10000

// The stack of the interpreter, literally a big array of floats
float stack[STACK_LIMIT];

// First FREE position in the stack
int head_pointer = 0;

int stack_push(float element)
{
    if (head_pointer >= STACK_LIMIT)
    {
        log_error("Stack size exceeded!\n");
        return 1;
    }
    stack[head_pointer] = element;
    head_pointer++;
    return 0;
}

int stack_pop(float* element)
{
    if (head_pointer == 0)
    {
        log_error("Stack empty!\n");
        return 1;
    }
    head_pointer--;
    *element = stack[head_pointer];
    return 0;
}

int stack_head(float* element)
{
    if (head_pointer == 0)
    {
        log_error("Tried to get the head of an empty stack!\n");
        return 1;
    }
    *element = stack[head_pointer - 1];
    return 0;
}

int stack_peek(float* element, int i)
{
    if (i < 0 || head_pointer < i)
    {
        log_error("Index %i out of stack range!\n", i);
        return 1;
    }
    *element = stack[i];
    return 0;
}

int stack_size(void)
{
    return head_pointer;
}

void stack_clear(void)
{
    head_pointer = 0;
}

