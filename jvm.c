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

const int32_t NULL_FOUR_BYTES = 0x0000;
const size_t TWO_OPERAND_OFFSET = 2;
const int32_t NEGATIVE_ONE = -1;
const int32_t ZERO = 0;
const int32_t ONE = 1;
const int32_t TWO = 2;
const int32_t THREE = 3;
const int32_t FOUR = 4;
const int32_t FIVE = 5;

/**
 * Represents the return value of a Java method: either void or an int or a reference.
 * For simplification, we represent a reference as an index into a heap-allocated
 * array. (In a real JVM, methods could also return object references or other
 * primitives.)
 */
typedef struct {
    /** Whether this returned value is an int */
    bool has_value;
    /** The returned value (only valid if `has_value` is true) */
    int32_t value;
} optional_value_t;

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
stack_t *stack_init() {
    // The stack  array is initially allocated to hold zero elements.
    stack_t *stack = calloc(1, sizeof(stack_t));
    // atm we use the heap to manage memory for stacks
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

void stack_print(stack_t *stack) {
    fprintf(stderr, "Printing Stack: \n [");
    for (size_t i = 0; i < stack->size; i++) {
        fprintf(stderr, "%d, ", stack->contents[i]);
    }
    fprintf(stderr, "]\n");
}

/**
 * Helper function to push the stack.
 */
int32_t stack_push(stack_t *stack, int32_t value) {
    if (stack->top + 1 < stack->size) {
        stack->contents[++(stack->top)] = value;
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
    if (stack->top > 0) {
        *value = stack->contents[stack->top];
        stack->contents[stack->top] = NULL_FOUR_BYTES;
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

__always_inline void iconst_helper(jvm_instruction_t opcode, stack_t *stack,
                                   size_t *program_counter) {
    // this instruction always increments the counter by 1
    (*program_counter)++;
    // we can take advantage of the opcode numbers to
    int32_t con = ((int32_t) opcode) - THREE;
    int32_t push_result = 0;
    push_result = stack_push(stack, con);
    assert(push_result == 1);
}

__always_inline void bipush_helper(stack_t *stack, size_t *program_counter,
                                   method_t *method) {
    // increment program counter by a total of two, one for the bipush instruction
    // itself, and another for the operand after pushing it to the stack.
    (*program_counter)++;
    // pushes the operand onto the stack.
    // remember the second program_counter increment here
    int32_t value = 0;
    // fprintf(stderr, "%08x", method-> code.code[(*program_counter)]);

    // the (signed char) cast is necessary to get negative values.
    value = (int32_t)((signed char) method->code.code[(*program_counter)++]);
    int32_t push_result = stack_push(stack, value);
    assert(push_result == 1);
}

__always_inline void sipush_helper(stack_t *stack, size_t *program_counter,
                                   method_t *method) {
    // increment program counter by a total of three, one for the sipush instruction
    // itself, and two for the two operands after pushing them on to the stack.
    (*program_counter)++;
    u1 first_operand = 0;
    u1 second_operand = 0;
    // remember the second program_counter increment here
    first_operand = method->code.code[(*program_counter)++];
    // remember the third program_counter increment here
    second_operand = method->code.code[(*program_counter)++];

    signed short result = 0;
    result = (((signed short) first_operand << 8) | second_operand);
    int32_t push_result = stack_push(stack, result);
    assert(push_result == 1);
}

__always_inline void constant_pool_helper(stack_t *stack, cp_info *pool_const) {
    assert(pool_const->info != NULL);

    switch (pool_const->tag) {
        case CONSTANT_Integer: {
            int32_t push_result = stack_push(
                stack, (int32_t)((CONSTANT_Integer_info *) pool_const->info)->bytes);
            assert(push_result == 1);
            break;
        }
        default: {
            break;
        }
    }
}

__always_inline void ldc_helper(stack_t *stack, size_t *program_counter, method_t *method,
                                class_file_t *class) {
    // load constant instruction
    // increment program counter by a total of two, one for the ldc instruction
    // itself, and another for the operand designating what index we should use to select
    // the constant we want to load.
    (*program_counter)++;

    size_t pool_index = 1;
    // the (unsigned char) cast is necessary to remember the operand in the code is
    // unsigned. remember the second program_counter increment here
    pool_index = (size_t)((unsigned char) method->code.code[(*program_counter)++]);
    cp_info pool_const = class->constant_pool[pool_index - 1];
    if (class->constant_pool[pool_index - 1].info != NULL) {
        constant_pool_helper(stack, &pool_const);
    }

    // int32_t push_result = stack_push(stack, class->constant_pool[pool_index - 1]);
    // assert(push_result == 1);
}

__always_inline void iload_helper(stack_t *stack, size_t *program_counter,
                                  method_t *method, int32_t *locals) {
    // Loads a local and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;

    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    int32_t push_result = stack_push(stack, locals[(size_t) first_operand]);
    assert(push_result == 1);
}

__always_inline void iload_n_helper(jvm_instruction_t opcode, stack_t *stack,
                                    size_t *program_counter, int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = opcode - i_iload_0;
    int32_t push_result = stack_push(stack, locals[locals_index]);
    assert(push_result == 1);
}

__always_inline void istore_helper(stack_t *stack, size_t *program_counter,
                                   method_t *method, int32_t *locals) {
    // stores an unsigned byte from the stack in the locals array at the place given by
    // the first operand.
    // increment the program_counter for the instruction itself
    (*program_counter)++;

    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    int32_t value = 0;
    int32_t pop_result = stack_pop(stack, &value);
    assert(pop_result == 1);

    locals[first_operand] = value;
}

__always_inline void istore_n_helper(jvm_instruction_t opcode, stack_t *stack,
                                     size_t *program_counter, int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = (size_t) opcode - i_istore_0;
    int32_t value = 0;
    int32_t pop_result = stack_pop(stack, &value);
    assert(pop_result == 1);

    locals[locals_index] = value;
}

__always_inline void iadd_helper(stack_t *stack, size_t *program_counter) {
    // addition instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand + second_operand);
    assert(push_result == 1);
}

__always_inline void isub_helper(stack_t *stack, size_t *program_counter) {
    // subtraction instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand - second_operand);
    assert(push_result == 1);
}

__always_inline void imul_helper(stack_t *stack, size_t *program_counter) {
    // multiplication instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand * second_operand);
    assert(push_result == 1);
}

__always_inline void idiv_helper(stack_t *stack, size_t *program_counter) {
    // multiplication instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand / second_operand);
    assert(push_result == 1);
}

__always_inline void irem_helper(stack_t *stack, size_t *program_counter) {
    // remainder instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand % second_operand);
    assert(push_result == 1);
}

__always_inline void ineg_helper(stack_t *stack, size_t *program_counter) {
    // negation instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t pop_result, push_result;
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, -first_operand);
    assert(push_result == 1);
}

__always_inline void ishl_helper(stack_t *stack, size_t *program_counter) {
    // signed bit shift left instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand << second_operand);
    assert(push_result == 1);
}

__always_inline void ishr_helper(stack_t *stack, size_t *program_counter) {
    // signed bit shift right instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand >> second_operand);
    assert(push_result == 1);
}

__always_inline void iushr_helper(stack_t *stack, size_t *program_counter) {
    // unsigned bit shift right
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, ((unsigned) first_operand) >> second_operand);
    assert(push_result == 1);
}

__always_inline void iand_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise AND instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand & second_operand);
    assert(push_result == 1);
}

__always_inline void ior_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise OR instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand | second_operand);
    assert(push_result == 1);
}

__always_inline void ixor_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise XOR instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    int32_t pop_result, push_result;
    // pop our second operand (pushed to the stack last).
    pop_result = stack_pop(stack, &second_operand);
    assert(pop_result == 1);
    // pop our first operand
    pop_result = stack_pop(stack, &first_operand);
    assert(pop_result == 1);
    // push the result back on to the stack.
    push_result = stack_push(stack, first_operand ^ second_operand);
    assert(push_result == 1);
}

__always_inline void iinc_helper(size_t *program_counter, method_t *method,
                                 int32_t *locals) {
    // increment program counter by a total of three, one for the iinc instruction
    // itself, and two for the two operands after pushing them on to the stack.
    (*program_counter)++;
    u1 first_operand = 0;
    int32_t second_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    // remember the third program_counter increment here
    // just like `bipush` and `sipush` we need to cast the second operand to a `signed
    // char` before casting to an int to get proper handling of negative values.
    second_operand = (int32_t)((signed char) method->code.code[(*program_counter)++]);

    locals[(size_t) first_operand] += (int32_t) second_operand;
}

__always_inline void return_helper(size_t *program_counter, method_t *method) {
    // return by setting the program counter to the end of the instruction list,
    // thus breaking the while loop
    *program_counter = method->code.code_length;
}

__always_inline void getstatic_helper(size_t *program_counter) {
    // increment the program counter for the `getstatic` opcode itself.
    (*program_counter)++;

    // increment it past the next two opcodes, for a total incrementation for each
    // use of this instruction of three, per the spec.
    (*program_counter) += TWO_OPERAND_OFFSET;
}

__always_inline void invokevirtual_helper(stack_t *stack, size_t *program_counter) {
    // invokevirtual b1 b2
    // Pops and prints the top value of the operand stack followed by a newline
    // character. Then, moves the program counter past b2 (i.e., increments it by
    // three).
    (*program_counter)++;

    int32_t pop_result, value;
    pop_result = stack_pop(stack, &value);
    fprintf(stdout, "%d\n", value);
    (*program_counter) += TWO_OPERAND_OFFSET;
}

// essentially a giant vtable that matches the opcode and handles running that
// instruction's implementation
void opcode_helper(stack_t *stack, size_t *program_counter, method_t *method,
                   int32_t *locals, class_file_t *class, heap_t *heap) {
    jvm_instruction_t opcode = (jvm_instruction_t) method->code.code[*program_counter];
    switch (opcode) {
        case i_nop: { // not implemented but its a no-op anyway
            break;
        }
        case i_iconst_m1: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_0: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_1: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_2: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_3: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_4: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_iconst_5: {
            iconst_helper(opcode, stack, program_counter);
            break;
        }
        case i_bipush: {
            bipush_helper(stack, program_counter, method);
            break;
        }
        case i_sipush: {
            sipush_helper(stack, program_counter, method);
            break;
        }
        case i_ldc: {
            ldc_helper(stack, program_counter, method, class);
            break;
        }
        case i_iload: {
            iload_helper(stack, program_counter, method, locals);
            break;
        }
        case i_iload_0: {
            iload_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_iload_1: {
            iload_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_iload_2: {
            iload_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_iload_3: {
            iload_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_istore: {
            istore_helper(stack, program_counter, method, locals);
            break;
        }
        case i_istore_0: {
            istore_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_istore_1: {
            istore_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_istore_2: {
            istore_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_istore_3: {
            istore_n_helper(opcode, stack, program_counter, locals);
            break;
        }
        case i_iadd: {
            iadd_helper(stack, program_counter);
            break;
        }
        case i_isub: {
            isub_helper(stack, program_counter);
            break;
        }
        case i_imul: {
            imul_helper(stack, program_counter);
            break;
        }
        case i_idiv: {
            idiv_helper(stack, program_counter);
            break;
        }
        case i_irem: {
            irem_helper(stack, program_counter);
            break;
        }
        case i_ineg: {
            ineg_helper(stack, program_counter);
            break;
        }
        case i_ishl: {
            ishl_helper(stack, program_counter);
            break;
        }
        case i_ishr: {
            ishr_helper(stack, program_counter);
            break;
        }
        case i_iushr: {
            iushr_helper(stack, program_counter);
            break;
        }
        case i_iand: {
            iand_helper(stack, program_counter);
            break;
        }
        case i_ior: {
            ior_helper(stack, program_counter);
            break;
        }
        case i_ixor: {
            ixor_helper(stack, program_counter);
            break;
        }
        case i_iinc: {
            iinc_helper(program_counter, method, locals);
            break;
        }
        case i_return: {
            return_helper(program_counter, method);
            break;
        }

        case i_getstatic: {
            getstatic_helper(program_counter);
            break;
        }

        case i_invokevirtual: {
            invokevirtual_helper(stack, program_counter);
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
