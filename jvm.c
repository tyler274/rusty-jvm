#include "jvm.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"
#include "opcodes.h"
#include "read_class.h"
#include "stack.h"

/** The name of the method to invoke to run the class file */
const char MAIN_METHOD[] = "main";
/**
 * The "descriptor" string for main(). The descriptor encodes main()'s signature,
 * i.e. main() takes a String[] and returns void.
 * If you're interested, the descriptor string is explained at
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-4.html#jvms-4.3.2.
 */
const char MAIN_DESCRIPTOR[] = "([Ljava/lang/String;)V";

// essentially a giant vtable that matches the opcode and handles running that
// instruction's implementation
void opcode_helper(stack_t *stack, size_t program_counter, method_t *method,
                   int32_t *locals, class_file_t *class, heap_t *heap,
                   optional_value_t *return_value) {
    jvm_instruction_t opcode = (jvm_instruction_t) method->code.code[program_counter];
    switch (opcode) {
        case i_nop:
            (program_counter)++;
            break;

        case i_iconst_m1:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_0:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_1:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_2:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_3:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_4:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_iconst_5:
            iconst_helper(opcode, stack, &program_counter);
            break;

        case i_bipush:
            bipush_helper(stack, &program_counter, method);
            break;

        case i_sipush:
            sipush_helper(stack, &program_counter, method);
            break;

        case i_ldc:
            ldc_helper(stack, &program_counter, method, class);
            break;

        case i_iload:
            iload_helper(stack, &program_counter, method, locals);
            break;

        case i_iload_0:
            iload_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_iload_1:
            iload_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_iload_2:
            iload_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_iload_3:
            iload_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_istore:
            istore_helper(stack, &program_counter, method, locals);
            break;

        case i_istore_0:
            istore_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_istore_1:
            istore_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_istore_2:
            istore_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_istore_3:
            istore_n_helper(opcode, stack, &program_counter, locals);
            break;

        case i_iadd:
            iadd_helper(stack, &program_counter);
            break;

        case i_isub:
            isub_helper(stack, &program_counter);
            break;

        case i_imul:
            imul_helper(stack, &program_counter);
            break;

        case i_idiv:
            idiv_helper(stack, &program_counter);
            break;

        case i_irem:
            irem_helper(stack, &program_counter);
            break;

        case i_ineg:
            ineg_helper(stack, &program_counter);
            break;

        case i_ishl:
            ishl_helper(stack, &program_counter);
            break;

        case i_ishr:
            ishr_helper(stack, &program_counter);
            break;

        case i_iushr:
            iushr_helper(stack, &program_counter);
            break;

        case i_iand:
            iand_helper(stack, &program_counter);
            break;

        case i_ior:
            ior_helper(stack, &program_counter);
            break;

        case i_ixor:
            ixor_helper(stack, &program_counter);
            break;

        case i_iinc:
            iinc_helper(&program_counter, method, locals);
            break;

        case i_ifeq:
            ifeq_helper(stack, &program_counter, method);
            break;

        case i_ifne:
            ifne_helper(stack, &program_counter, method);
            break;

        case i_iflt:
            iflt_helper(stack, &program_counter, method);
            break;

        case i_ifge:
            ifge_helper(stack, &program_counter, method);
            break;

        case i_ifgt:
            ifgt_helper(stack, &program_counter, method);
            break;

        case i_ifle:
            ifle_helper(stack, &program_counter, method);
            break;

        case i_if_icmpeq:
            if_icmpeq_helper(stack, &program_counter, method);
            break;

        case i_if_icmpne:
            if_icmpne_helper(stack, &program_counter, method);
            break;

        case i_if_icmplt:
            if_icmplt_helper(stack, &program_counter, method);
            break;

        case i_if_icmpge:
            if_icmpge_helper(stack, &program_counter, method);
            break;

        case i_if_icmpgt:
            if_icmpgt_helper(stack, &program_counter, method);
            break;

        case i_if_icmple:
            if_icmple_helper(stack, &program_counter, method);
            break;

        case i_goto:
            goto_helper(&program_counter, method);
            break;

        case i_ireturn:
            ireturn_helper(stack, &program_counter, method, return_value);
            break;

        case i_return:
            return_helper(&program_counter, method);
            break;

        case i_getstatic:
            getstatic_helper(&program_counter);
            break;

        case i_invokevirtual:
            invokevirtual_helper(stack, &program_counter);
            break;

        case i_invokestatic: {
            (program_counter)++;

            u1 first_operand = 0;
            u1 second_operand = 0;
            // remember the second and third program_counter increments here
            first_operand = method->code.code[(program_counter)++];
            second_operand = method->code.code[(program_counter)++];
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

            optional_value_t returned_value =
                execute(sub_method, locals_ptr, class, heap);
            if (returned_value.has_value == true) {
                int32_t push_result = stack_push(stack, returned_value.value);
                assert(push_result == 1);
            }
            // invokestatic_helper(stack, &program_counter, method, class, heap);
            break;
        }
        default:
            fprintf(stderr, "Running unimplemented opcode: %d\n\n", opcode);
            assert(false);
            (program_counter)++;
            break;
    }
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
    // heap_add(heap, stack_ptr);

    stack_t *stack = stack_init();
    stack->size = method->code.max_stack;
    // assign the stack a pointer on the heap.
    stack->contents = stack_ptr;

    // Return void
    optional_value_t result = {.has_value = false};

    while (program_counter < method->code.code_length) {
        // opcode_helper(stack, &program_counter, method, locals, class, heap, &result);
        jvm_instruction_t opcode = (jvm_instruction_t) method->code.code[program_counter];
        switch (opcode) {
            case i_nop:
                (program_counter)++;
                break;

            case i_iconst_m1:
                iconst_helper(opcode, stack, &program_counter);
                break;

            case i_iconst_0:
            case i_iconst_1:
            case i_iconst_2:
            case i_iconst_3:
            case i_iconst_4:
            case i_iconst_5:
                iconst_helper(opcode, stack, &program_counter);
                break;

            case i_bipush:
                bipush_helper(stack, &program_counter, method);
                break;

            case i_sipush:
                sipush_helper(stack, &program_counter, method);
                break;

            case i_ldc:
                ldc_helper(stack, &program_counter, method, class);
                break;

            case i_iload:
                iload_helper(stack, &program_counter, method, locals);
                break;
            case i_aload:
                aload_helper();
                break;
            case i_iload_0:
            case i_iload_1:
            case i_iload_2:
            case i_iload_3:
                iload_n_helper(opcode, stack, &program_counter, locals);
                break;
            case i_aload_0:
            case i_aload_1:
            case i_aload_2:
            case i_aload_3:
                aload_n_helper();
                break;

            case i_iaload:
                iaload_helper();
                break;
            case i_istore:
                istore_helper(stack, &program_counter, method, locals);
                break;
            case i_astore:
                astore_helper();
                break;

            case i_istore_0:
            case i_istore_1:
            case i_istore_2:
            case i_istore_3:
                istore_n_helper(opcode, stack, &program_counter, locals);
                break;
            case i_astore_0:
            case i_astore_1:
            case i_astore_2:
            case i_astore_3:
                astore_n_helper(opcode, stack, &program_counter, locals);
                break;
            case i_iastore:
                iastore_helper();
                break;
            case i_dup:
                dup_helper(stack, &program_counter);
                break;
            case i_iadd:
                iadd_helper(stack, &program_counter);
                break;

            case i_isub:
                isub_helper(stack, &program_counter);
                break;

            case i_imul:
                imul_helper(stack, &program_counter);
                break;

            case i_idiv:
                idiv_helper(stack, &program_counter);
                break;

            case i_irem:
                irem_helper(stack, &program_counter);
                break;

            case i_ineg:
                ineg_helper(stack, &program_counter);
                break;

            case i_ishl:
                ishl_helper(stack, &program_counter);
                break;

            case i_ishr:
                ishr_helper(stack, &program_counter);
                break;

            case i_iushr:
                iushr_helper(stack, &program_counter);
                break;

            case i_iand:
                iand_helper(stack, &program_counter);
                break;

            case i_ior:
                ior_helper(stack, &program_counter);
                break;

            case i_ixor:
                ixor_helper(stack, &program_counter);
                break;

            case i_iinc:
                iinc_helper(&program_counter, method, locals);
                break;

            case i_ifeq:
                ifeq_helper(stack, &program_counter, method);
                break;

            case i_ifne:
                ifne_helper(stack, &program_counter, method);
                break;

            case i_iflt:
                iflt_helper(stack, &program_counter, method);
                break;

            case i_ifge:
                ifge_helper(stack, &program_counter, method);
                break;

            case i_ifgt:
                ifgt_helper(stack, &program_counter, method);
                break;

            case i_ifle:
                ifle_helper(stack, &program_counter, method);
                break;

            case i_if_icmpeq:
                if_icmpeq_helper(stack, &program_counter, method);
                break;

            case i_if_icmpne:
                if_icmpne_helper(stack, &program_counter, method);
                break;

            case i_if_icmplt:
                if_icmplt_helper(stack, &program_counter, method);
                break;

            case i_if_icmpge:
                if_icmpge_helper(stack, &program_counter, method);
                break;

            case i_if_icmpgt:
                if_icmpgt_helper(stack, &program_counter, method);
                break;

            case i_if_icmple:
                if_icmple_helper(stack, &program_counter, method);
                break;

            case i_goto:
                goto_helper(&program_counter, method);
                break;

            case i_ireturn:
                ireturn_helper(stack, &program_counter, method, &result);
                break;

            case i_areturn:
                areturn_helper();
                break;

            case i_return:
                return_helper(&program_counter, method);
                break;

            case i_getstatic:
                getstatic_helper(&program_counter);
                break;

            case i_invokevirtual:
                invokevirtual_helper(stack, &program_counter);
                break;

            case i_invokestatic: {
                (program_counter)++;

                u1 first_operand = 0;
                u1 second_operand = 0;
                // remember the second and third program_counter increments here
                first_operand = method->code.code[(program_counter)++];
                second_operand = method->code.code[(program_counter)++];
                u2 sub_method_index = (first_operand << 8) | second_operand;
                method_t *sub_method = find_method_from_index(sub_method_index, class);
                assert(sub_method != NULL);

                int32_t *locals_ptr =
                    calloc(sub_method->code.max_locals, sizeof(int32_t));
                u2 num_args = get_number_of_parameters(sub_method);

                int32_t pop_result = 0;
                int32_t popped_value = 0;
                for (size_t i = 0; i < num_args; i++) {
                    pop_result = stack_pop(stack, &popped_value);
                    assert(pop_result == 1);
                    locals_ptr[num_args - (i + 1)] = popped_value;
                }

                optional_value_t returned_value =
                    execute(sub_method, locals_ptr, class, heap);
                if (returned_value.has_value == true) {
                    int32_t push_result = stack_push(stack, returned_value.value);
                    assert(push_result == 1);
                }
                free(locals_ptr);
                // invokestatic_helper(stack, &program_counter, method, class, heap);
                break;
            }

            case i_newarray:
                newarray_helper();
                break;

            case i_arraylength:
                arraylength_helper();
                break;
            default:
                fprintf(stderr, "Running unimplemented opcode: %d\n\n", opcode);
                assert(false);
                (program_counter)++;
                break;
        }
    }

    stack_free(stack);

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
