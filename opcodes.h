#ifndef OPCODES_H
#define OPCODES_H

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

void iconst_helper(jvm_instruction_t opcode, stack_t *stack, size_t *program_counter) {
    // this instruction always increments the counter by 1
    (*program_counter)++;
    // we can take advantage of the opcode numbers to calculate the constant value
    assert(stack_push(stack, ((int32_t) opcode) - THREE) == 1);
}

void bipush_helper(stack_t *stack, size_t *program_counter, method_t *method) {
    // increment program counter by a total of two, one for the bipush instruction
    // itself, and another for the operand after pushing it to the stack.
    (*program_counter)++;
    // pushes the operand onto the stack.
    // remember the second program_counter increment here
    // fprintf(stderr, "%08x", method-> code.code[(*program_counter)]);

    // the (signed char) cast is necessary to get negative values.
    assert(stack_push(stack,
                      (int32_t)((signed char) method->code.code[(*program_counter)++])) ==
           1);
}

void sipush_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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
    assert(stack_push(stack, result) == 1);
}

// switches on the constant type to determine how we should process pool_const's info
// field.
void constant_pool_helper(stack_t *stack, cp_info *pool_const) {
    assert(pool_const->info != NULL);

    switch (pool_const->tag) {
        case CONSTANT_Integer: {
            int32_t cast_int =
                (int32_t)((CONSTANT_Integer_info *) pool_const->info)->bytes;
            assert(stack_push(stack, cast_int) == 1);
            break;
        }
        default: {
            break;
        }
    }
}

void ldc_helper(stack_t *stack, size_t *program_counter, method_t *method,
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
}

void iload_helper(stack_t *stack, size_t *program_counter, method_t *method,
                  int32_t *locals) {
    // Loads a local and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    assert(stack_push(stack, locals[(size_t) first_operand]) == 1);
}

void aload_helper(stack_t *stack, size_t *program_counter, method_t *method,
                  int32_t *locals) {
    // Loads a local and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    // remember the second program_counter increment here
    assert(
        stack_push(
            stack,
            locals[(size_t)((unsigned char) method->code.code[(*program_counter)++])]) ==
        1);
}

void iload_n_helper(jvm_instruction_t opcode, stack_t *stack, size_t *program_counter,
                    int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    assert(stack_push(stack, locals[opcode - i_iload_0]) == 1);
}

void aload_n_helper(jvm_instruction_t opcode, stack_t *stack, size_t *program_counter,
                    int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    assert(stack_push(stack, locals[opcode - i_aload_0]) == 1);
}

void iaload_helper(stack_t *stack, size_t *program_counter, heap_t *heap) {
    (*program_counter)++;
    int32_t index = 0;
    assert(stack_pop(stack, &index) == 1);
    int32_t reference = 0;
    assert(stack_pop(stack, &reference) == 1);
    int32_t *array = heap_get(heap, reference);
    assert(index < array[0]);
    int32_t value = array[index + 1];
    assert(stack_push(stack, value) == 1);
}

void istore_helper(stack_t *stack, size_t *program_counter, method_t *method,
                   int32_t *locals) {
    // stores an unsigned byte from the stack in the locals array at the place given by
    // the first operand.
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    int32_t value = 0;
    assert(stack_pop(stack, &value) == 1);
    locals[first_operand] = value;
}

void astore_helper(stack_t *stack, size_t *program_counter, method_t *method,
                   int32_t *locals) {
    // stores an unsigned byte from the stack in the locals array at the place given by
    // the first operand.
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    u1 first_operand = 0;
    // remember the second program_counter increment here
    first_operand = (unsigned char) method->code.code[(*program_counter)++];
    int32_t reference = 0;
    assert(stack_pop(stack, &reference) == 1);
    locals[first_operand] = reference;
}

void istore_n_helper(jvm_instruction_t opcode, stack_t *stack, size_t *program_counter,
                     int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = (size_t) opcode - i_istore_0;
    int32_t value = 0;
    assert(stack_pop(stack, &value) == 1);
    locals[locals_index] = value;
}

void astore_n_helper(jvm_instruction_t opcode, stack_t *stack, size_t *program_counter,
                     int32_t *locals) {
    // Loads a local_n instruction where n in {0,1,2,3,4} and pushes it onto the stack
    // increment the program_counter for the instruction itself
    (*program_counter)++;
    size_t locals_index = (size_t) opcode - i_astore_0;
    int32_t reference = 0;
    assert(stack_pop(stack, &reference) == 1);
    locals[locals_index] = reference;
}

void iastore_helper(stack_t *stack, size_t *program_counter, heap_t *heap) {
    (*program_counter)++;
    int32_t value = 0;
    assert(stack_pop(stack, &value) == 1);
    int32_t index = 0;
    assert(stack_pop(stack, &index) == 1);
    int32_t reference = 0;
    assert(stack_pop(stack, &reference) == 1);
    int32_t *array = heap_get(heap, reference);
    assert(index < array[0]);
    array[index + 1] = value;
}

void dup_helper(stack_t *stack, size_t *program_counter) {
    (*program_counter)++;
    int32_t value = 0;
    assert(stack_pop(stack, &value) == 1);
    assert(stack_push(stack, value) == 1);
    assert(stack_push(stack, value) == 1);
}

void iadd_helper(stack_t *stack, size_t *program_counter) {
    // addition instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand + second_operand) == 1);
}

void isub_helper(stack_t *stack, size_t *program_counter) {
    // subtraction instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand - second_operand) == 1);
}

void imul_helper(stack_t *stack, size_t *program_counter) {
    // multiplication instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand * second_operand) == 1);
}

void idiv_helper(stack_t *stack, size_t *program_counter) {
    // multiplication instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand / second_operand) == 1);
}

void irem_helper(stack_t *stack, size_t *program_counter) {
    // remainder instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand % second_operand) == 1);
}

void ineg_helper(stack_t *stack, size_t *program_counter) {
    // negation instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, -first_operand) == 1);
}

void ishl_helper(stack_t *stack, size_t *program_counter) {
    // signed bit shift left instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    // pop_result = stack_pop(stack, &first_operand);
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    // push_result = stack_push(stack, first_operand << second_operand);
    assert(stack_push(stack, first_operand << second_operand) == 1);
}

void ishr_helper(stack_t *stack, size_t *program_counter) {
    // signed bit shift right instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand >> second_operand) == 1);
}

void iushr_helper(stack_t *stack, size_t *program_counter) {
    // unsigned bit shift right
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, ((unsigned) first_operand) >> second_operand) == 1);
}

void iand_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise AND instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand & second_operand) == 1);
}

void ior_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise OR instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand | second_operand) == 1);
}

void ixor_helper(stack_t *stack, size_t *program_counter) {
    // bit-wise XOR instruction
    // increment the program counter by one, as add doesn't take any operands
    (*program_counter)++;
    int32_t first_operand = 0;
    int32_t second_operand = 0;
    // pop our second operand (pushed to the stack last).
    assert(stack_pop(stack, &second_operand) == 1);
    // pop our first operand
    assert(stack_pop(stack, &first_operand) == 1);
    // push the result back on to the stack.
    assert(stack_push(stack, first_operand ^ second_operand) == 1);
}

void iinc_helper(size_t *program_counter, method_t *method, int32_t *locals) {
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

int32_t jump_offset_helper(size_t *program_counter, method_t *method) {
    int8_t first_operand = 0;
    int8_t second_operand = 0;
    // remember the second program_counter increment here
    first_operand = (signed char) method->code.code[(*program_counter)++];
    // remember the third program_counter increment here
    // just like `bipush` and `sipush` we need to cast the second operand to an `unsigned
    // char`.
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

void jump_one_op_helper(stack_t *stack, size_t *program_counter, method_t *method,
                        int32_t *first_stack_operand, int32_t *jump_offset) {
    (*program_counter)++;
    // Calculate the offset
    *jump_offset = jump_offset_helper(program_counter, method);
    // pop the first stack operand off the top of the stack
    assert(stack_pop(stack, first_stack_operand) == 1);
}

void jump_two_ops_helper(stack_t *stack, size_t *program_counter, method_t *method,
                         int32_t *first_stack_operand, int32_t *second_stack_operand,
                         int32_t *jump_offset) {
    (*program_counter)++;
    *jump_offset = jump_offset_helper(program_counter, method);
    // pop the second stack operand off the top of the stack
    assert(stack_pop(stack, second_stack_operand) == 1);
    // pop the first stack operand off the top of the stack
    assert(stack_pop(stack, first_stack_operand) == 1);
}

void ifeq_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void ifne_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void iflt_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void ifge_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void ifgt_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void ifle_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void if_icmpeq_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void if_icmpne_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void if_icmplt_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void if_icmpge_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void if_icmpgt_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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
void if_icmple_helper(stack_t *stack, size_t *program_counter, method_t *method) {
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

void goto_helper(size_t *program_counter, method_t *method) {
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

void ireturn_helper(stack_t *stack, size_t *program_counter, method_t *method,
                    optional_value_t *result) {
    (*program_counter)++;
    int32_t value = 0;
    assert(stack_pop(stack, &value) == 1);

    result->has_value = true;
    result->value = value;

    // move the program counter to the end of the method so the outer while loop breaks.
    *program_counter = method->code.code_length;
}

void return_helper(size_t *program_counter, method_t *method) {
    // return by setting the program counter to the end of the instruction list,
    // thus breaking the while loop
    *program_counter = method->code.code_length;
}

void getstatic_helper(size_t *program_counter) {
    // increment the program counter for the `getstatic` opcode itself.
    (*program_counter)++;

    // increment it past the next two opcodes, for a total incrementation for each
    // use of this instruction of three, per the spec.
    (*program_counter) += TWO_OPERAND_OFFSET;
}

void invokevirtual_helper(stack_t *stack, size_t *program_counter) {
    // invokevirtual b1 b2
    // Pops and prints the top value of the operand stack followed by a newline
    // character. Then, moves the program counter past b2 (i.e., increments it by
    // three).
    (*program_counter)++;

    int32_t value = 0;
    // pop_result = stack_pop(stack, &value);
    assert(stack_pop(stack, &value) == 1);
    fprintf(stdout, "%d\n", value);
    (*program_counter) += TWO_OPERAND_OFFSET;
}

void not_implemented_helper(size_t *program_counter, jvm_instruction_t *opcode) {
    (*program_counter)++;
    fprintf(stderr, "Running unimplemented opcode: %d\n\n", *opcode);
    (*program_counter)++;
    assert(false);
}

// this function needs to be always inlined into opcode_helper() and from there execute()
// or the stack will overflow on the Recursion test.
__attribute__((always_inline)) inline void invokestatic_helper(stack_t *stack,
                                                               size_t *program_counter,
                                                               method_t *method,
                                                               class_file_t *class,
                                                               heap_t *heap) {
    (*program_counter)++;
    // the instruction takes two operands from the two opcodes that follow it in the
    // code array. It then recursively executes the submethod indexed by the fusion of
    // those two operand unsigned bytes.
    u1 first_operand = 0;
    u1 second_operand = 0;
    // remember the second and third program_counter increments here
    first_operand = method->code.code[(*program_counter)++];
    second_operand = method->code.code[(*program_counter)++];

    // fuse the unsigned bytes to get an index that we can use to get a pointer to the sub
    // method we want to recursively execute.
    u2 sub_method_index = (first_operand << 8) | second_operand;
    method_t *sub_method = find_method_from_index(sub_method_index, class);

    // double check that the sub method we got isn't NULL
    assert(sub_method != NULL);

    // the caller of execute needs to allocate the local array using the information
    // contained in the sub method's code's max_locals variable.
    int32_t *locals_ptr = calloc(sub_method->code.max_locals, sizeof(int32_t));

    // to find out how many arguments there are to the submethod we call this helper
    // function and store the result in an unsigned short (uint16).
    u2 num_args = get_number_of_parameters(sub_method);

    // for each argument to the sub method we pop the argument's value off the stack and
    // then insert it into our sub method's locals array.
    int32_t popped_value = 0;
    for (size_t i = 0; i < num_args; i++) {
        assert(stack_pop(stack, &popped_value) == 1);
        locals_ptr[num_args - (i + 1)] = popped_value;
    }

    // execute our sub method by recursively calling execute.
    optional_value_t returned_value = execute(sub_method, locals_ptr, class, heap);

    // if our sub method has a return value, we push that value onto the stack.
    if (returned_value.has_value == true) {
        assert(stack_push(stack, returned_value.value) == 1);
    }
    // after our sub method returns we can free its locals array's memory. The stack for
    // our sub method is initialized inside the execute function.
    free(locals_ptr);
}

void newarray_helper(stack_t *stack, size_t *program_counter, method_t *method,
                     heap_t *heap) {
    (*program_counter)++;
    // creates a new int32_t array and stores it on the heap.

    // the operand to this opcode is just the type of the array, in our cases it will
    // always be '10' to indicate its a signed 32bit integer array.
    u1 first_operand = method->code.code[(*program_counter)++];
    assert(first_operand == 10);
    // get the size of the new array by popping the count value off the stack.
    int32_t count = 0;
    assert(stack_pop(stack, &count) == 1);
    // allocate and initialize an array of size 'count + 1', we use the first member of
    // the array to store the array's size.
    int32_t *new_array = calloc(count + 1, sizeof(int32_t));
    // we store the size of the array as an additional entry at the front.
    new_array[0] = count;
    // add the new array to the heap, and then push the returned reference to it onto the
    // stack.
    assert(stack_push(stack, heap_add(heap, new_array)) == 1);
}

void arraylength_helper(stack_t *stack, size_t *program_counter, heap_t *heap) {
    (*program_counter)++;
    // pops a reference to an array on the heap off of the stack and then pushes that
    // array's length back on to the stack.
    int32_t reference = 0;
    assert(stack_pop(stack, &reference) == 1);
    int32_t *array = heap_get(heap, reference);
    // We store the size of the array in the first entry.
    assert(stack_push(stack, array[0]) == 1);
}

void areturn_helper(stack_t *stack, size_t *program_counter, method_t *method,
                    optional_value_t *return_value) {
    (*program_counter)++;
    // returns a reference to an array.
    int32_t reference = 0;
    // pop the reference to the array off of the stack and then set the return value to
    // it.
    assert(stack_pop(stack, &reference) == 1);
    return_value->has_value = true;
    return_value->value = reference;

    // move the program counter to the end of the method's code to break the while loop
    // and return.
    (*program_counter) = method->code.code_length;
}

// essentially a giant vtable that matches the opcode and handles running that
// instruction's implementation
// this function needs to be always inlined into execute()
// or the stack will overflow on the Recursion test.
__attribute__((always_inline)) inline void opcode_helper(
    stack_t *stack, size_t *program_counter, method_t *method, int32_t *locals,
    class_file_t *class, heap_t *heap, optional_value_t *result) {
    jvm_instruction_t opcode = (jvm_instruction_t) method->code.code[*program_counter];
    switch (opcode) {
        case i_nop:
            (program_counter)++;
            break;

        case i_iconst_m1:
        case i_iconst_0:
        case i_iconst_1:
        case i_iconst_2:
        case i_iconst_3:
        case i_iconst_4:
        case i_iconst_5:
            iconst_helper(opcode, stack, program_counter);
            break;

        case i_bipush:
            bipush_helper(stack, program_counter, method);
            break;

        case i_sipush:
            sipush_helper(stack, program_counter, method);
            break;

        case i_ldc:
            ldc_helper(stack, program_counter, method, class);
            break;

        case i_iload:
            iload_helper(stack, program_counter, method, locals);
            break;
        case i_aload:
            aload_helper(stack, program_counter, method, locals);
            break;
        case i_iload_0:
        case i_iload_1:
        case i_iload_2:
        case i_iload_3:
            iload_n_helper(opcode, stack, program_counter, locals);
            break;
        case i_aload_0:
        case i_aload_1:
        case i_aload_2:
        case i_aload_3:
            aload_n_helper(opcode, stack, program_counter, locals);
            break;

        case i_iaload:
            iaload_helper(stack, program_counter, heap);
            break;
        case i_istore:
            istore_helper(stack, program_counter, method, locals);
            break;
        case i_astore:
            astore_helper(stack, program_counter, method, locals);
            break;

        case i_istore_0:
        case i_istore_1:
        case i_istore_2:
        case i_istore_3:
            istore_n_helper(opcode, stack, program_counter, locals);
            break;
        case i_astore_0:
        case i_astore_1:
        case i_astore_2:
        case i_astore_3:
            astore_n_helper(opcode, stack, program_counter, locals);
            break;
        case i_iastore:
            iastore_helper(stack, program_counter, heap);
            break;
        case i_dup:
            dup_helper(stack, program_counter);
            break;
        case i_iadd:
            iadd_helper(stack, program_counter);
            break;

        case i_isub:
            isub_helper(stack, program_counter);
            break;

        case i_imul:
            imul_helper(stack, program_counter);
            break;

        case i_idiv:
            idiv_helper(stack, program_counter);
            break;

        case i_irem:
            irem_helper(stack, program_counter);
            break;

        case i_ineg:
            ineg_helper(stack, program_counter);
            break;

        case i_ishl:
            ishl_helper(stack, program_counter);
            break;

        case i_ishr:
            ishr_helper(stack, program_counter);
            break;

        case i_iushr:
            iushr_helper(stack, program_counter);
            break;

        case i_iand:
            iand_helper(stack, program_counter);
            break;

        case i_ior:
            ior_helper(stack, program_counter);
            break;

        case i_ixor:
            ixor_helper(stack, program_counter);
            break;

        case i_iinc:
            iinc_helper(program_counter, method, locals);
            break;

        case i_ifeq:
            ifeq_helper(stack, program_counter, method);
            break;

        case i_ifne:
            ifne_helper(stack, program_counter, method);
            break;

        case i_iflt:
            iflt_helper(stack, program_counter, method);
            break;

        case i_ifge:
            ifge_helper(stack, program_counter, method);
            break;

        case i_ifgt:
            ifgt_helper(stack, program_counter, method);
            break;

        case i_ifle:
            ifle_helper(stack, program_counter, method);
            break;

        case i_if_icmpeq:
            if_icmpeq_helper(stack, program_counter, method);
            break;

        case i_if_icmpne:
            if_icmpne_helper(stack, program_counter, method);
            break;

        case i_if_icmplt:
            if_icmplt_helper(stack, program_counter, method);
            break;

        case i_if_icmpge:
            if_icmpge_helper(stack, program_counter, method);
            break;

        case i_if_icmpgt:
            if_icmpgt_helper(stack, program_counter, method);
            break;

        case i_if_icmple:
            if_icmple_helper(stack, program_counter, method);
            break;

        case i_goto:
            goto_helper(program_counter, method);
            break;

        case i_ireturn:
            ireturn_helper(stack, program_counter, method, result);
            break;

        case i_return:
            return_helper(program_counter, method);
            break;

        case i_getstatic:
            getstatic_helper(program_counter);
            break;

        case i_invokevirtual:
            invokevirtual_helper(stack, program_counter);
            break;

        case i_invokestatic:
            invokestatic_helper(stack, program_counter, method, class, heap);
            break;

        case i_newarray:
            newarray_helper(stack, program_counter, method, heap);
            break;

        case i_arraylength:
            arraylength_helper(stack, program_counter, heap);
            break;

        case i_areturn:
            areturn_helper(stack, program_counter, method, result);
            break;
        default:
            not_implemented_helper(program_counter, &opcode);
            break;
    }
}

#endif /* OPCODES_H */