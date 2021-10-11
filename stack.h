#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const int32_t NULL_FOUR_BYTES = 0x0000;

// simple stack implementation
typedef struct {
    // the maximum size of the stack
    size_t size;
    // the index of the current top of the stack
    size_t top;
    // the callee must allocate this on the heap for now, and free it using the associated
    // function.
    int32_t *contents;
} stack_t;

// init a stack struct in memory and return a pointer to it
stack_t *stack_init(size_t size) {
    // The stack  array is initially allocated to hold zero elements.
    stack_t *stack = calloc(1, sizeof(stack_t));
    // atm we use the heap to manage memory for stacks
    // stack->contents = NULL;
    stack->contents = (int32_t *) calloc(size, sizeof(int32_t));
    stack->size = size;
    stack->top = 0;
    return stack;
}

void stack_free(stack_t *stack) {
    // We handle this ourselves now, still watch out for double free.
    free(stack->contents);
    free(stack);
}

/**
 * Helper function to print out the stack
 */
void stack_print(stack_t *stack) {
    fprintf(stderr, "Printing Stack: size=%zu, top=%zu\n [", stack->size, stack->top);
    for (size_t i = 0; i < stack->size; i++) {
        fprintf(stderr, "%d, ", stack->contents[i]);
    }
    fprintf(stderr, "]\n");
}

bool stack_is_empty(stack_t *stack) {
    if (stack->top == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool stack_is_full(stack_t *stack) {
    if (stack->top >= stack->size) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Helper function to push the stack.
 */

int32_t stack_push(stack_t *stack, int32_t value) {
    if (!stack_is_full(stack)) {
        stack->contents[stack->top] = value;
        // if stack->top == 0
        stack->top += 1;
        // return 1 if the value was added to the stack
        return 1;
    }
    else {
        // Oops Stack Overflow
        fprintf(stderr, "Error: Stack Overflow, value: %d\n", value);
        stack_print(stack);
        return 0;
    }
}

int32_t stack_pop(stack_t *stack, int32_t *value) {
    if (!stack_is_empty(stack)) {
        (*value) = stack->contents[stack->top - 1];
        stack->contents[stack->top - 1] = NULL_FOUR_BYTES;
        stack->top--;
        // return 1 if something is popped off
        return 1;
    }
    else {
        // return 0 if the stack is empty
        // Stack Underflow;
        fprintf(stderr, "Error: Stack Underflow\n");
        stack_print(stack);
        return 0;
    }
}

#endif /* STACK_H */