#ifndef JVM_H
#define JVM_H

#include <inttypes.h>
#include <stdbool.h>

#include "class_file.h"
#include "heap.h"

/**
 * JVM integer instruction mnemonics and opcodes. If you're interested,
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-6.html
 * has the full list of JVM instructions and their specifications.
 */
typedef enum {
    i_nop = 0x0,
    i_iconst_m1 = 0x2,
    i_iconst_0 = 0x3,
    i_iconst_1 = 0x4,
    i_iconst_2 = 0x5,
    i_iconst_3 = 0x6,
    i_iconst_4 = 0x7,
    i_iconst_5 = 0x8,
    i_bipush = 0x10,
    i_sipush = 0x11,
    i_ldc = 0x12,
    i_iload = 0x15,
    i_aload = 0x19,
    i_iload_0 = 0x1a,
    i_iload_1 = 0x1b,
    i_iload_2 = 0x1c,
    i_iload_3 = 0x1d,
    i_aload_0 = 0x2a,
    i_aload_1 = 0x2b,
    i_aload_2 = 0x2c,
    i_aload_3 = 0x2d,
    i_iaload = 0x2e,
    i_istore = 0x36,
    i_astore = 0x3a,
    i_istore_0 = 0x3b,
    i_istore_1 = 0x3c,
    i_istore_2 = 0x3d,
    i_istore_3 = 0x3e,
    i_astore_0 = 0x4b,
    i_astore_1 = 0x4c,
    i_astore_2 = 0x4d,
    i_astore_3 = 0x4e,
    i_iastore = 0x4f,
    i_dup = 0x59,
    i_iadd = 0x60,
    i_isub = 0x64,
    i_imul = 0x68,
    i_idiv = 0x6c,
    i_irem = 0x70,
    i_ineg = 0x74,
    i_ishl = 0x78,
    i_ishr = 0x7a,
    i_iushr = 0x7c,
    i_iand = 0x7e,
    i_ior = 0x80,
    i_ixor = 0x82,
    i_iinc = 0x84,
    i_ifeq = 0x99,
    i_ifne = 0x9a,
    i_iflt = 0x9b,
    i_ifge = 0x9c,
    i_ifgt = 0x9d,
    i_ifle = 0x9e,
    i_if_icmpeq = 0x9f,
    i_if_icmpne = 0xa0,
    i_if_icmplt = 0xa1,
    i_if_icmpge = 0xa2,
    i_if_icmpgt = 0xa3,
    i_if_icmple = 0xa4,
    i_goto = 0xa7,
    i_ireturn = 0xac,
    i_areturn = 0xb0,
    i_return = 0xb1,
    i_getstatic = 0xb2,
    i_invokevirtual = 0xb6,
    i_invokestatic = 0xb8,
    i_newarray = 0xbc,
    i_arraylength = 0xbe
} jvm_instruction_t;

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

optional_value_t execute(method_t *method, int32_t *locals, class_file_t *class,
                         heap_t *heap);

#endif /* JVM_H */
