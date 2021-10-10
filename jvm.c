#include "jvm.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"
#include "read_class.h"

/** The name of the method to invoke to run the class file */
const char MAIN_METHOD[] = "main";
/**
 * The "descriptor" string for main(). The descriptor encodes main()'s signature,
 * i.e. main() takes a String[] and returns void.
 * If you're interested, the descriptor string is explained at
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-4.html#jvms-4.3.2.
 */
const char MAIN_DESCRIPTOR[] = "([Ljava/lang/String;)V";

const int32_t null_four_bytes = 0x0000;

/**
 * Represents the return value of a Java method: either void or an int or a reference.
 * For simplification, we represent a reference as an index into a heap-allocated array.
 * (In a real JVM, methods could also return object references or other primitives.)
 */
typedef struct {
    /** Whether this returned value is an int */
    bool has_value;
    /** The returned value (only valid if `has_value` is true) */
    int32_t value;
} optional_value_t;

typedef struct {
    size_t size;
    size_t top;
    int32_t *contents;
} stack_t;

stack_t *stack_init() {
    // The stack  array is initially allocated to hold zero elements.
    stack_t *stack = calloc(1, sizeof(stack_t));
    stack->contents = NULL;
    stack->size = 0;
    stack->top = 0;
    return stack;
}

void stack_free(stack_t *stack) {
    /**
     *  At the moment the stack's contents are allocated on the heap, and to avoid double
     * free UB should be freed there as well.
     */
    // free(stack->contents);
    free(stack);
}

/**
 * Helper function to push the stack.
 */
int stack_push(stack_t *stack, int32_t value) {
    if (stack->top + 1 < stack->size) {
        stack->contents[++stack->top] = value;
        // return 1 if the value was added to the stack
        return 1;
    }
    else {
        // Oops Stack Overflow
        return 0;
    }
}

int stack_pop(stack_t *stack, int32_t *value) {
    if (stack->top > 0) {
        *value = stack->contents[stack->top];
        stack->contents[stack->top] = null_four_bytes;
        stack->top--;
        // return 1 if something is popped off
        return 1;
    }
    else {
        // return 0 if the stack is empty
        // Stack Underflow;
        return 0;
    }
}

void opcode_helper(stack_t *stack, size_t *program_counter, method_t *method,
                   int32_t *locals, class_file_t *class, heap_t *heap) {
    switch ((jvm_instruction_t) method->code.code[*program_counter]) {
        case i_bipush: {
            *program_counter += 1;
            int push_result = stack_push(stack, method->code.code[*program_counter]);
            assert(push_result == 1);
            /* code */
            break;
        }
        case i_iadd: {
            int32_t first_operand, second_operand;
            int pop_result, push_result;
            pop_result = stack_pop(stack, &second_operand);
            assert(pop_result == 1);
            pop_result = stack_pop(stack, &first_operand);
            assert(pop_result == 1);
            push_result = stack_push(stack, first_operand + second_operand);
            assert(push_result == 1);
            /* code */
            break;
        }
        case i_return: {
            /* code */
            *program_counter = method->code.code_length;
            break;
        }

        case i_nop: {
            break;
        }
        default: {
            break;
        }
    }

    (void) locals;
    (void) class;
    (void) heap;
}

/**
 * Runs a method's instructions until the method returns.
 *
 * @param method the method to run
 * @param locals the array of local variables, including the method parameters.
 *   Except for parameters, the locals are uninitialized.
 * @param class the class file the method belongs to
 * @param heap an array of heap-allocated pointers, useful for references
 * @return an optional int containing the method's return value
 */
optional_value_t execute(method_t *method, int32_t *locals, class_file_t *class,
                         heap_t *heap) {
    size_t program_counter = 0;
    int32_t *stack_ptr = calloc(method->code.max_stack, sizeof(int32_t));
    heap_add(heap, stack_ptr);

    stack_t *stack = stack_init();
    stack->size = method->code.max_stack;
    // assign the stack a pointer on the heap.
    stack->contents = stack_ptr;

    while (program_counter < method->code.code_length) {
        opcode_helper(stack, &program_counter, method, locals, class, heap);
    }

    /* You should remove these casts to void in your solution.
     * They are just here so the code compiles without warnings. */
    // (void) method;
    (void) locals;
    (void) class;
    // (void) heap;
    // (void) program_counter;

    stack_free(stack);

    // Return void
    optional_value_t result = {.has_value = false};
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <class file>\n", argv[0]);
        return 1;
    }

    // Open the class file for reading
    FILE *class_file = fopen(argv[1], "r");
    assert(class_file != NULL && "Failed to open file");

    // Parse the class file
    class_file_t *class = get_class(class_file);
    int error = fclose(class_file);
    assert(error == 0 && "Failed to close file");

    // The heap array is initially allocated to hold zero elements.
    heap_t *heap = heap_init();

    // Execute the main method
    method_t *main_method = find_method(MAIN_METHOD, MAIN_DESCRIPTOR, class);
    assert(main_method != NULL && "Missing main() method");
    /* In a real JVM, locals[0] would contain a reference to String[] args.
     * But since TeenyJVM doesn't support Objects, we leave it uninitialized. */
    int32_t locals[main_method->code.max_locals];
    // Initialize all local variables to 0
    memset(locals, 0, sizeof(locals));
    optional_value_t result = execute(main_method, locals, class, heap);
    assert(!result.has_value && "main() should return void");

    // Free the internal data structures
    free_class(class);

    // Free the heap
    heap_free(heap);
}
