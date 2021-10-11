#pragma once
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvm.h"
#include "read_class.h"
#include "stack.h"

const size_t TWO_OPERAND_OFFSET = 2;
const int32_t NEGATIVE_ONE = -1;
const int32_t ZERO = 0;
const int32_t ONE = 1;
const int32_t TWO = 2;
const int32_t THREE = 3;
const int32_t FOUR = 4;
const int32_t FIVE = 5;

typedef struct {
    size_t size;
    size_t position;
    int32_t *contents;
} array_t;

// init a stack struct in memory and return a pointer to it
array_t *array_init() {
    // The stack  array is initially allocated to hold zero elements.
    array_t *array = calloc(1, sizeof(array_t));
    // atm we use the heap to manage memory for stacks
    array->contents = NULL;
    array->size = 0;
    array->position = 0;
    return array;
}

void array_free(array_t *array) {
    /**
     *  At the moment the stack's contents are allocated on the heap, and to avoid double
     * free UB should be freed there as well.
     */
    free(array->contents);
    free(array);
}

void array_print(array_t *array) {
    fprintf(stderr, "Printing array: size=%zu, position=%zu\n [", array->size,
            array->position);
    for (size_t i = 0; i < array->size; i++) {
        fprintf(stderr, "%d, ", array->contents[i]);
    }
    fprintf(stderr, "]\n");
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

// switches on the constant type to determine how we should process pool_const's info
// field.
__always_inline void constant_pool_helper(stack_t *stack, cp_info *pool_const) {
    assert(pool_const->info != NULL);

    switch (pool_const->tag) {
        case CONSTANT_Integer: {
            int32_t cast_int =
                (int32_t)((CONSTANT_Integer_info *) pool_const->info)->bytes;
            int32_t push_result = stack_push(stack, cast_int);
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

__always_inline void aload_helper(stack_t *stack, size_t *program_counter,
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

__always_inline void aload_n_helper(jvm_instruction_t opcode, stack_t *stack,
                                    size_t *program_counter, int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = opcode - i_aload_0;
    int32_t push_result = stack_push(stack, locals[locals_index]);
    assert(push_result == 1);
}

__always_inline void iaload_helper(stack_t *stack, size_t *program_counter,
                                   heap_t *heap) {
    (*program_counter)++;

    int32_t index = 0;
    int32_t pop_result = stack_pop(stack, &index);
    assert(pop_result == 1);
    int32_t reference = 0;
    pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);

    int32_t *array = heap_get(heap, reference);
    assert(index < array[0]);
    int32_t value = array[index + 1];
    int32_t push_result = stack_push(stack, value);
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

__always_inline void astore_helper(stack_t *stack, size_t *program_counter,
                                   method_t *method, int32_t *locals) {
    // stores an unsigned byte from the stack in the locals array at the place given by
    // the first operand.
    // increment the program_counter for the instruction itself
    (*program_counter)++;

    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    int32_t reference = 0;
    int32_t pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);

    locals[first_operand] = reference;
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

__always_inline void astore_n_helper(jvm_instruction_t opcode, stack_t *stack,
                                     size_t *program_counter, int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = (size_t) opcode - i_astore_0;
    int32_t reference = 0;
    int32_t pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);

    locals[locals_index] = reference;
}

__always_inline void iastore_helper(stack_t *stack, size_t *program_counter,
                                    heap_t *heap) {
    (*program_counter)++;
    int32_t value = 0;
    int32_t pop_result = stack_pop(stack, &value);
    assert(pop_result == 1);
    int32_t index = 0;
    pop_result = stack_pop(stack, &index);
    assert(pop_result == 1);
    int32_t reference = 0;
    pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);

    int32_t *array = heap_get(heap, reference);
    assert(index < array[0]);
    array[index + 1] = value;
}

__always_inline void dup_helper(stack_t *stack, size_t *program_counter) {
    (*program_counter)++;
    int32_t value = 0;
    int32_t pop_result = stack_pop(stack, &value);
    assert(pop_result == 1);
    int32_t push_result = 0;
    push_result = stack_push(stack, value);
    assert(push_result == 1);
    push_result = stack_push(stack, value);
    assert(push_result == 1);
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

// how many times the program_counter is incremented that we need to account for when
// calculating the offset.
const int32_t JUMP_TWO_OPCODES_OFFSET = -3;

__always_inline int32_t jump_offset_helper(size_t *program_counter, method_t *method) {
    int8_t first_operand = 0;
    int8_t second_operand = 0;
    // remember the second program_counter increment here
    // first_operand = (unsigned char) method->code.code[(*program_counter)++];
    first_operand = (signed char) method->code.code[(*program_counter)++];
    // remember the third program_counter increment here
    // just like `bipush` and `sipush` we need to cast the second operand to an `unsigned
    // char`.
    // second_operand = (unsigned char) method->code.code[(*program_counter)++];
    second_operand = (signed char) method->code.code[(*program_counter)++];

    int32_t jump_offset = 0;
    jump_offset =
        ((int16_t)((int16_t)((signed char) first_operand) << 8) | second_operand) +
        JUMP_TWO_OPCODES_OFFSET;

    // fprintf(stderr,
    //         "Jump Debugging Assistant\nprogram_counter: %zu\nfirst operand byte: "
    //         "%x\nsecond operand byte: "
    //         "%x\njump_offset: %d\nprogram_counter after offset: %zu\n\n",
    //         *program_counter, first_operand, second_operand, jump_offset,
    //         (*program_counter) + jump_offset);
    return jump_offset;
}

__always_inline void jump_one_op_helper(stack_t *stack, size_t *program_counter,
                                        method_t *method, int32_t *first_stack_operand,
                                        int32_t *jump_offset) {
    (*program_counter)++;
    *jump_offset = jump_offset_helper(program_counter, method);

    int32_t pop_result = 0;
    // pop the first stack operand off the top of the stack
    pop_result = stack_pop(stack, first_stack_operand);
    assert(pop_result == 1);
}

__always_inline void jump_two_ops_helper(stack_t *stack, size_t *program_counter,
                                         method_t *method, int32_t *first_stack_operand,
                                         int32_t *second_stack_operand,
                                         int32_t *jump_offset) {
    (*program_counter)++;
    *jump_offset = jump_offset_helper(program_counter, method);

    int32_t pop_result = 0;
    // pop the second stack operand off the top of the stack
    pop_result = stack_pop(stack, second_stack_operand);
    assert(pop_result == 1);
    // pop the first stack operand off the top of the stack
    pop_result = stack_pop(stack, first_stack_operand);
    assert(pop_result == 1);
}

__always_inline void ifeq_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if equal to zero, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is 0, and if it is we increment the program counter by
    // the signed offset given by the fusion of the first and second operands.
    if (first_stack_operand == 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void ifne_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if not equal to 0, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is NOT EQUAL to 0, and if it is we increment the
    // program counter by the signed offset given by the fusion of the first and second
    // operands.
    if (first_stack_operand != 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void iflt_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if less than zero, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is LESS THAN 0, and if it is we increment the
    // program counter by the signed offset given by the fusion of the first and second
    // operands.
    if (first_stack_operand < 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void ifge_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if greater than or equal to zero, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is GREATER THAN OR EQUAL to 0, and if it is we
    // increment the program counter by the signed offset given by the fusion of the first
    // and second operands.
    if (first_stack_operand >= 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void ifgt_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if greater than zero, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is GREATER THAN 0, and if it is we increment the
    // program counter by the signed offset given by the fusion of the first and second
    // operands.
    if (first_stack_operand > 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void ifle_helper(stack_t *stack, size_t *program_counter,
                                 method_t *method) {
    // `if less than or equal to zero, then jump` instruction
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, jump_offset;
    first_stack_operand = 0;
    jump_offset = 0;
    jump_one_op_helper(stack, program_counter, method, &first_stack_operand,
                       &jump_offset);

    // We test if the popped value is LESS THAN OR EQUAL TO 0, and if it is we increment
    // the program counter by the signed offset given by the fusion of the first and
    // second operands.
    if (first_stack_operand <= 0) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void if_icmpeq_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is  equal to the
    // second stack operand (top of the stack, first to pop), then jump` instruction
    // increment program counter by a total of three, one for the `if_icmpeq` instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is less than or equal to the second stack
    // operand, and if it is we increment the program counter by the signed offset given
    // by the fusion of the first and second operands.
    if (first_stack_operand == second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void if_icmpne_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is not equal to the
    // second stack operand (top of the stack, first to pop), then jump` instruction
    // increment program counter by a total of three, one for the `if_icmpne` instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is less than or equal to the second stack
    // operand, and if it is we increment the program counter by the signed offset given
    // by the fusion of the first and second operands.
    if (first_stack_operand != second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void if_icmplt_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is less than the
    // second stack operand (top of the stack, first to pop), then jump` instruction
    // increment program counter by a total of three, one for the `if_icmplt` instruction
    // itself, and two for the two operands after pushing them on to the stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is less than or equal to the second stack
    // operand, and if it is we increment the program counter by the signed offset given
    // by the fusion of the first and second operands.
    if (first_stack_operand < second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void if_icmpge_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is greater than or
    // equal to the second stack operand (top of the stack, first to pop), then jump`
    // instruction increment program counter by a total of three, one for the `if_icmpge`
    // instruction itself, and two for the two operands after pushing them on to the
    // stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is less than or equal to the second stack
    // operand, and if it is we increment the program counter by the signed offset
    // given by the fusion of the first and second operands.
    if (first_stack_operand >= second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void if_icmpgt_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is greater than
    // the second stack operand (top of the stack, first to pop), then jump`
    // instruction increment program counter by a total of three, one for the if_icmpgt
    // instruction itself, and two for the two operands after pushing them on to the
    // stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is greater than the second stack
    // operand, and if it is we increment the program counter by the signed offset given
    // by the fusion of the first and second operands.
    if (first_stack_operand > second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}
__always_inline void if_icmple_helper(stack_t *stack, size_t *program_counter,
                                      method_t *method) {
    // `if the first stack operand (second to the top of the stack) is less than or equal
    // to the second stack operand (top of the stack, first to pop), then jump`
    // instruction increment program counter by a total of three, one for the if_icmple
    // instruction itself, and two for the two operands after pushing them on to the
    // stack.
    int32_t first_stack_operand, second_stack_operand, jump_offset;
    first_stack_operand = 0;
    second_stack_operand = 0;
    jump_offset = 0;
    jump_two_ops_helper(stack, program_counter, method, &first_stack_operand,
                        &second_stack_operand, &jump_offset);

    // We test if the first stack operand is less than or equal to the second stack
    // operand, and if it is we increment the program counter by the signed offset given
    // by the fusion of the first and second operands.
    if (first_stack_operand <= second_stack_operand) {
        (*program_counter) += jump_offset;
    }
}

__always_inline void goto_helper(size_t *program_counter, method_t *method) {
    // goto instruction, increments the program counter to jump to a specific part of the
    // method's code.
    // increment program counter by a total of three, one for the ifeq instruction
    // itself, and two for the two operands after pushing them on to the stack.
    (*program_counter)++;
    int32_t jump_offset = 0;
    jump_offset = jump_offset_helper(program_counter, method);

    // We increment the
    // program counter by the signed offset given by the fusion of the first and second
    // operands.
    (*program_counter) += jump_offset;
}

__always_inline void ireturn_helper(stack_t *stack, size_t *program_counter,
                                    method_t *method, optional_value_t *result) {
    (*program_counter)++;
    int32_t pop_result = 0;
    int32_t value = 0;
    pop_result = stack_pop(stack, &value);
    assert(pop_result == 1);

    result->has_value = true;
    result->value = value;

    *program_counter = method->code.code_length;
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

static inline void invokestatic_helper(stack_t *stack, size_t *program_counter,
                                       method_t *method, class_file_t *class,
                                       heap_t *heap) {
    (*program_counter)++;

    u1 first_operand = 0;
    u1 second_operand = 0;
    // remember the second and third program_counter increments here
    first_operand = method->code.code[(*program_counter)++];
    second_operand = method->code.code[(*program_counter)++];
    u2 sub_method_index = (first_operand << 8) | second_operand;
    method_t *sub_method = find_method_from_index(sub_method_index, class);
    assert(sub_method != NULL);

    int32_t *locals_ptr = calloc(sub_method->code.max_locals, sizeof(int32_t));
    heap_add(heap, locals_ptr);
    u2 num_args = get_number_of_parameters(sub_method);

    int32_t pop_result = 0;
    int32_t popped_value = 0;
    for (size_t i = 0; i < num_args; i++) {
        pop_result = stack_pop(stack, &popped_value);
        assert(pop_result == 1);
        locals_ptr[num_args - (i + 1)] = popped_value;
    }

    optional_value_t returned_value = execute(sub_method, locals_ptr, class, heap);
    if (returned_value.has_value == true) {
        int32_t push_result = stack_push(stack, returned_value.value);
        assert(push_result == 1);
    }
}

__always_inline void newarray_helper(stack_t *stack, size_t *program_counter,
                                     method_t *method, heap_t *heap) {
    (*program_counter)++;
    u1 first_operand = method->code.code[(*program_counter)++];
    assert(first_operand == 10);

    int32_t pop_result = 0;
    int32_t push_result = 0;
    int32_t count = 0;
    pop_result = stack_pop(stack, &count);
    assert(pop_result == 1);
    int32_t *new_array = calloc(count + 1, sizeof(int32_t));
    // we store the size of the array as an additional entry at the front.
    new_array[0] = count;
    push_result = stack_push(stack, heap_add(heap, new_array));
    assert(push_result == 1);
}

__always_inline void arraylength_helper(stack_t *stack, size_t *program_counter,
                                        heap_t *heap) {
    (*program_counter)++;

    int32_t pop_result = 0;
    int32_t push_result = 0;
    int32_t reference = 0;
    pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);
    int32_t *array = heap_get(heap, reference);
    push_result = stack_push(stack, array[0]);
    assert(push_result == 1);
}

__always_inline void areturn_helper(stack_t *stack, size_t *program_counter,
                                    method_t *method, optional_value_t *return_value) {
    (*program_counter)++;

    int32_t reference = 0;
    int32_t pop_result = 0;

    pop_result = stack_pop(stack, &reference);
    assert(pop_result == 1);

    return_value->has_value = true;
    return_value->value = reference;

    (*program_counter) = method->code.code_length;
}