use std::ops::Deref;

use crate::read_class::{
    class_file_t, code_t, cp_info, cp_info_t, cp_info_tag, find_method, find_method_from_index,
    get_class, get_number_of_parameters, method_t,
};
use crate::stack::{
    stack_init, stack_is_empty, stack_is_full, stack_pop, stack_print, stack_push, stack_t,
};

pub const TWO_OPERAND_OFFSET: usize = 2;

// pub const NEGATIVE_ONE: i32 = -1;

// pub const ZERO: i32 = 0;

// pub const ONE: i32 = 1;

// pub const TWO: i32 = 2;

pub const THREE: i32 = 3;

// pub const FOUR: i32 = 4;

// pub const FIVE: i32 = 5;

pub type heap_t = Vec<Vec<i32>>;

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum jvm_instruction_t {
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
    i_arraylength = 0xbe,
}

impl std::convert::TryFrom<u8> for jvm_instruction_t {
    type Error = ();

    fn try_from(v: u8) -> Result<Self, Self::Error> {
        match v {
            0x0 => Ok(jvm_instruction_t::i_nop),
            0x2 => Ok(jvm_instruction_t::i_iconst_m1),
            0x3 => Ok(jvm_instruction_t::i_iconst_0),
            0x4 => Ok(jvm_instruction_t::i_iconst_1),
            0x5 => Ok(jvm_instruction_t::i_iconst_2),
            0x6 => Ok(jvm_instruction_t::i_iconst_3),
            0x7 => Ok(jvm_instruction_t::i_iconst_4),
            0x8 => Ok(jvm_instruction_t::i_iconst_5),
            0x10 => Ok(jvm_instruction_t::i_bipush),
            0x11 => Ok(jvm_instruction_t::i_sipush),
            0x12 => Ok(jvm_instruction_t::i_ldc),
            0x15 => Ok(jvm_instruction_t::i_iload),
            0x19 => Ok(jvm_instruction_t::i_aload),
            0x1a => Ok(jvm_instruction_t::i_iload_0),
            0x1b => Ok(jvm_instruction_t::i_iload_1),
            0x1c => Ok(jvm_instruction_t::i_iload_2),
            0x1d => Ok(jvm_instruction_t::i_iload_3),
            0x2a => Ok(jvm_instruction_t::i_aload_0),
            0x2b => Ok(jvm_instruction_t::i_aload_1),
            0x2c => Ok(jvm_instruction_t::i_aload_2),
            0x2d => Ok(jvm_instruction_t::i_aload_3),
            0x2e => Ok(jvm_instruction_t::i_iaload),
            0x36 => Ok(jvm_instruction_t::i_istore),
            0x3a => Ok(jvm_instruction_t::i_astore),
            0x3b => Ok(jvm_instruction_t::i_istore_0),
            0x3c => Ok(jvm_instruction_t::i_istore_1),
            0x3d => Ok(jvm_instruction_t::i_istore_2),
            0x3e => Ok(jvm_instruction_t::i_istore_3),
            0x4b => Ok(jvm_instruction_t::i_astore_0),
            0x4c => Ok(jvm_instruction_t::i_astore_1),
            0x4d => Ok(jvm_instruction_t::i_astore_2),
            0x4e => Ok(jvm_instruction_t::i_astore_3),
            0x4f => Ok(jvm_instruction_t::i_iastore),
            0x59 => Ok(jvm_instruction_t::i_dup),
            0x60 => Ok(jvm_instruction_t::i_iadd),
            0x64 => Ok(jvm_instruction_t::i_isub),
            0x68 => Ok(jvm_instruction_t::i_imul),
            0x6c => Ok(jvm_instruction_t::i_idiv),
            0x70 => Ok(jvm_instruction_t::i_irem),
            0x74 => Ok(jvm_instruction_t::i_ineg),
            0x78 => Ok(jvm_instruction_t::i_ishl),
            0x7a => Ok(jvm_instruction_t::i_ishr),
            0x7c => Ok(jvm_instruction_t::i_iushr),
            0x7e => Ok(jvm_instruction_t::i_iand),
            0x80 => Ok(jvm_instruction_t::i_ior),
            0x82 => Ok(jvm_instruction_t::i_ixor),
            0x84 => Ok(jvm_instruction_t::i_iinc),
            0x99 => Ok(jvm_instruction_t::i_ifeq),
            0x9a => Ok(jvm_instruction_t::i_ifne),
            0x9b => Ok(jvm_instruction_t::i_iflt),
            0x9c => Ok(jvm_instruction_t::i_ifge),
            0x9d => Ok(jvm_instruction_t::i_ifgt),
            0x9e => Ok(jvm_instruction_t::i_ifle),
            0x9f => Ok(jvm_instruction_t::i_if_icmpeq),
            0xa0 => Ok(jvm_instruction_t::i_if_icmpne),
            0xa1 => Ok(jvm_instruction_t::i_if_icmplt),
            0xa2 => Ok(jvm_instruction_t::i_if_icmpge),
            0xa3 => Ok(jvm_instruction_t::i_if_icmpgt),
            0xa4 => Ok(jvm_instruction_t::i_if_icmple),
            0xa7 => Ok(jvm_instruction_t::i_goto),
            0xac => Ok(jvm_instruction_t::i_ireturn),
            0xb0 => Ok(jvm_instruction_t::i_areturn),
            0xb1 => Ok(jvm_instruction_t::i_return),
            0xb2 => Ok(jvm_instruction_t::i_getstatic),
            0xb6 => Ok(jvm_instruction_t::i_invokevirtual),
            0xb8 => Ok(jvm_instruction_t::i_invokestatic),
            0xbc => Ok(jvm_instruction_t::i_newarray),
            0xbe => Ok(jvm_instruction_t::i_arraylength),
            _ => Err(()),
        }
    }
}

impl std::convert::TryFrom<i32> for jvm_instruction_t {
    type Error = ();

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            0x0 => Ok(jvm_instruction_t::i_nop),
            0x2 => Ok(jvm_instruction_t::i_iconst_m1),
            0x3 => Ok(jvm_instruction_t::i_iconst_0),
            0x4 => Ok(jvm_instruction_t::i_iconst_1),
            0x5 => Ok(jvm_instruction_t::i_iconst_2),
            0x6 => Ok(jvm_instruction_t::i_iconst_3),
            0x7 => Ok(jvm_instruction_t::i_iconst_4),
            0x8 => Ok(jvm_instruction_t::i_iconst_5),
            0x10 => Ok(jvm_instruction_t::i_bipush),
            0x11 => Ok(jvm_instruction_t::i_sipush),
            0x12 => Ok(jvm_instruction_t::i_ldc),
            0x15 => Ok(jvm_instruction_t::i_iload),
            0x19 => Ok(jvm_instruction_t::i_aload),
            0x1a => Ok(jvm_instruction_t::i_iload_0),
            0x1b => Ok(jvm_instruction_t::i_iload_1),
            0x1c => Ok(jvm_instruction_t::i_iload_2),
            0x1d => Ok(jvm_instruction_t::i_iload_3),
            0x2a => Ok(jvm_instruction_t::i_aload_0),
            0x2b => Ok(jvm_instruction_t::i_aload_1),
            0x2c => Ok(jvm_instruction_t::i_aload_2),
            0x2d => Ok(jvm_instruction_t::i_aload_3),
            0x2e => Ok(jvm_instruction_t::i_iaload),
            0x36 => Ok(jvm_instruction_t::i_istore),
            0x3a => Ok(jvm_instruction_t::i_astore),
            0x3b => Ok(jvm_instruction_t::i_istore_0),
            0x3c => Ok(jvm_instruction_t::i_istore_1),
            0x3d => Ok(jvm_instruction_t::i_istore_2),
            0x3e => Ok(jvm_instruction_t::i_istore_3),
            0x4b => Ok(jvm_instruction_t::i_astore_0),
            0x4c => Ok(jvm_instruction_t::i_astore_1),
            0x4d => Ok(jvm_instruction_t::i_astore_2),
            0x4e => Ok(jvm_instruction_t::i_astore_3),
            0x4f => Ok(jvm_instruction_t::i_iastore),
            0x59 => Ok(jvm_instruction_t::i_dup),
            0x60 => Ok(jvm_instruction_t::i_iadd),
            0x64 => Ok(jvm_instruction_t::i_isub),
            0x68 => Ok(jvm_instruction_t::i_imul),
            0x6c => Ok(jvm_instruction_t::i_idiv),
            0x70 => Ok(jvm_instruction_t::i_irem),
            0x74 => Ok(jvm_instruction_t::i_ineg),
            0x78 => Ok(jvm_instruction_t::i_ishl),
            0x7a => Ok(jvm_instruction_t::i_ishr),
            0x7c => Ok(jvm_instruction_t::i_iushr),
            0x7e => Ok(jvm_instruction_t::i_iand),
            0x80 => Ok(jvm_instruction_t::i_ior),
            0x82 => Ok(jvm_instruction_t::i_ixor),
            0x84 => Ok(jvm_instruction_t::i_iinc),
            0x99 => Ok(jvm_instruction_t::i_ifeq),
            0x9a => Ok(jvm_instruction_t::i_ifne),
            0x9b => Ok(jvm_instruction_t::i_iflt),
            0x9c => Ok(jvm_instruction_t::i_ifge),
            0x9d => Ok(jvm_instruction_t::i_ifgt),
            0x9e => Ok(jvm_instruction_t::i_ifle),
            0x9f => Ok(jvm_instruction_t::i_if_icmpeq),
            0xa0 => Ok(jvm_instruction_t::i_if_icmpne),
            0xa1 => Ok(jvm_instruction_t::i_if_icmplt),
            0xa2 => Ok(jvm_instruction_t::i_if_icmpge),
            0xa3 => Ok(jvm_instruction_t::i_if_icmpgt),
            0xa4 => Ok(jvm_instruction_t::i_if_icmple),
            0xa7 => Ok(jvm_instruction_t::i_goto),
            0xac => Ok(jvm_instruction_t::i_ireturn),
            0xb0 => Ok(jvm_instruction_t::i_areturn),
            0xb1 => Ok(jvm_instruction_t::i_return),
            0xb2 => Ok(jvm_instruction_t::i_getstatic),
            0xb6 => Ok(jvm_instruction_t::i_invokevirtual),
            0xb8 => Ok(jvm_instruction_t::i_invokestatic),
            0xbc => Ok(jvm_instruction_t::i_newarray),
            0xbe => Ok(jvm_instruction_t::i_arraylength),
            _ => Err(()),
        }
    }
}

pub fn iconst_helper(opcode: jvm_instruction_t, stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // assert!(stack_push(stack, (opcode as i32) - THREE));
    stack.push((opcode as i32) - THREE);
}

pub fn bipush_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh0 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    // the (signed char) cast is necessary to get negative values in the C code.
    let value = (method.deref().code.code[fresh0] as i8) as i32;
    stack.push(value);
}

pub fn sipush_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh1 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh1 as usize];
    let fresh2 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: u8 = (*method).code.code[fresh2];
    let result: i16 = ((first_operand as i16) << 8_i32 | second_operand as i16) as i16;
    // assert!(stack_push(stack, result as i32));
    stack.push(result as i32);
}

pub fn constant_pool_helper(stack: &mut Vec<i32>, pool_const: &cp_info) {
    assert!(pool_const.info.is_some());
    match pool_const.tag {
        cp_info_tag::CONSTANT_Integer => match pool_const.info.as_deref() {
            Some(cp_info_t::CONSTANT_Integer_info { bytes }) => {
                // assert!(stack_push(stack, *bytes as i32))
                stack.push(*bytes);
            }
            _ => {
                eprintln!("Expected an integer");
                std::process::exit(1);
            }
        },
        _ => {}
    };
}

pub fn ldc_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    class: &class_file_t,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh3 = *program_counter;
    // remember the second program_counter increment here
    *program_counter = (*program_counter).wrapping_add(1);
    // the (unsigned char) cast is necessary to remember the operand in the code is
    // unsigned.
    let pool_index: usize = ((*method).code.code[fresh3] as u8) as usize;

    let pool_const: cp_info = (*class).constant_pool[pool_index.wrapping_sub(1)].clone();
    if (*class).constant_pool[pool_index.wrapping_sub(1) as usize]
        .info
        .is_some()
    {
        constant_pool_helper(stack, &pool_const);
    };
}

pub fn iload_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh4 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh4] as u8;
    // assert!(stack_push(stack, locals[first_operand as usize]));
    stack.push(locals[first_operand as usize]);
}

pub fn aload_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh5 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    // assert!(stack_push(
    //     stack,
    //     locals[(*method).code.code[fresh5] as usize],
    // ));
    stack.push(locals[(method.deref().code.code[fresh5] as u8) as usize])
}

pub fn iload_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    // assert!(stack_push(
    //     stack,
    //     locals[(opcode as i32 as usize).wrapping_sub(jvm_instruction_t::i_iload_0 as usize)]
    // ));
    stack
        .push(locals[(opcode as i32 as usize).wrapping_sub(jvm_instruction_t::i_iload_0 as usize)]);
}

pub fn aload_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    // assert!(stack_push(
    //     stack,
    //     locals[(opcode as usize).wrapping_sub(jvm_instruction_t::i_aload_0 as usize) as usize]
    // ));
    stack.push(
        locals
            [(opcode as i32 as usize).wrapping_sub(jvm_instruction_t::i_aload_0 as usize) as usize],
    );
}

pub fn iaload_helper(stack: &mut Vec<i32>, program_counter: &mut usize, heap: &mut heap_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut index: i32 = 0;
    // assert!(stack_pop(stack, &mut index));
    let index: i32 = stack.pop().unwrap();
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));
    let reference: i32 = stack.pop().unwrap();
    let array: &Vec<i32> = &heap[reference as usize];
    assert!((index as usize) < array.len());
    let value: i32 = array[index as usize];
    // assert!(stack_push(stack, value));
    stack.push(value);
}

pub fn istore_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh6 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = method.deref().code.code[fresh6];
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    let value = stack.pop().unwrap();
    assert!((first_operand as usize) < locals.len());
    locals[first_operand as usize] = value;
}

pub fn astore_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh7 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh7];
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));
    let reference = stack.pop().unwrap();
    assert!((first_operand as usize) < locals.len());
    locals[first_operand as usize] = reference;
}

pub fn istore_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let locals_index: usize =
        (opcode as usize).wrapping_sub(jvm_instruction_t::i_istore_0 as usize);
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    let value = stack.pop().unwrap();
    assert!(locals_index < locals.len());
    locals[locals_index] = value;
}

pub fn astore_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let locals_index: usize =
        (opcode as usize).wrapping_sub(jvm_instruction_t::i_astore_0 as usize);
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));
    let reference = stack.pop().unwrap();
    assert!(locals_index < locals.len());
    locals[locals_index] = reference;
}

pub fn iastore_helper(stack: &mut Vec<i32>, program_counter: &mut usize, heap: &mut heap_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    let value = stack.pop().unwrap();
    // let mut index: i32 = 0;
    // assert!(stack_pop(stack, &mut index));
    let index = stack.pop().unwrap();
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));
    let reference = stack.pop().unwrap();
    assert!((reference as usize) < heap.len());
    let array: &mut Vec<i32> = &mut heap[reference as usize];
    assert!((index as usize) < array.len());
    //NOTE: found that I needed to add 1 to the index here due to the dummy value and zero
    //indexing I think?
    array[index as usize] = value;
}

pub fn dup_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    let value = stack.pop().unwrap();
    // assert!(stack_push(stack, value));
    stack.push(value);
    // assert!(stack_push(stack, value));
    stack.push(value);
}

pub fn iadd_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(
    //     stack,
    //     first_operand.wrapping_add(second_operand)
    // ));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    stack.push(first_operand.wrapping_add(second_operand));
}

pub fn isub_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(
    //     stack,
    //     first_operand.wrapping_sub(second_operand)
    // ));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    stack.push(first_operand.wrapping_sub(second_operand));
}

pub fn imul_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(
    //     stack,
    //     first_operand.wrapping_mul(second_operand)
    // ));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    stack.push(first_operand.wrapping_mul(second_operand));
}

pub fn vec_print_helper(stack: &Vec<i32>) -> String {
    format!("{:?}", *stack)
}

pub fn idiv_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, second_operand / first_operand));

    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("idiv First Operand: {first_operand}, Second Operand: {second_operand}");
    if second_operand == 0 {
        // format!("{:?}", stack);
        eprintln!("idiv First Operand: {first_operand}, Second Operand: {second_operand}");
        assert!(false);
    }
    // assert!(second_operand != 0);

    stack.push(first_operand.wrapping_div(second_operand));
}

pub fn irem_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, second_operand % first_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("irem First Operand: {first_operand}, Second Operand: {second_operand}");
    assert!(second_operand != 0);
    stack.push(first_operand.wrapping_rem(second_operand));
}

pub fn ineg_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand.wrapping_neg()));
    let first_operand = stack.pop().unwrap();
    stack.push(first_operand.wrapping_neg());
}

pub fn ishl_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand << second_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("ishl First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(first_operand << second_operand);
}

pub fn ishr_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand >> second_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("ishr First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(first_operand >> second_operand);
}

pub fn iushr_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(
    //     stack,
    //     (first_operand as u32 >> second_operand) as i32,
    // ));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("iushr First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(((first_operand as u32) >> second_operand) as i32);
}

pub fn iand_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    // bit-wise AND instruction
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand & second_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("iand First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(first_operand & second_operand);
}

pub fn ior_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    // bit-wise OR instruction
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand | second_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("ior First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(first_operand | second_operand);
}

pub fn ixor_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    // bit-wise XOR instruction
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut first_operand: i32 = 0;
    // let mut second_operand: i32 = 0;
    // assert!(stack_pop(stack, &mut second_operand));
    // assert!(stack_pop(stack, &mut first_operand));
    // assert!(stack_push(stack, first_operand ^ second_operand));
    let second_operand = stack.pop().unwrap();
    let first_operand = stack.pop().unwrap();
    // eprintln!("ior First Operand: {first_operand}, Second Operand: {second_operand}");
    stack.push(first_operand ^ second_operand);
}

pub fn iinc_helper(program_counter: &mut usize, method: &method_t, locals: &mut Vec<i32>) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh8 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh8];
    let fresh9 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    // remember the third program_counter increment here
    // just like `bipush` and `sipush` we need to cast the second operand to a `signed
    // char` before casting to an int to get proper handling of negative values.
    let second_operand: i32 = (method.deref().code.code[fresh9] as i8) as i32;
    locals[first_operand as usize] += second_operand as i32;
}

// how many times the program_counter is incremented that we need to account for when
// calculating the offset.
pub const JUMP_TWO_OPCODES_OFFSET: i32 = -3;

pub fn jump_offset_helper(program_counter: &mut usize, method: &method_t) -> i32 {
    let fresh11 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    // just like `bipush` and `sipush` we need to cast the second operand to an `unsigned
    // char`.
    let first_operand: i8 = method.deref().code.code[fresh11] as i8;
    let fresh12 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: i8 = method.deref().code.code[fresh12] as i8;
    let jump_offset: i32 = (((((first_operand as i8) as i16) << 8_i32) as i16)
        | second_operand as i16) as i32
        + JUMP_TWO_OPCODES_OFFSET;
    // TODO: Check casting behavior here vs C.
    // eprintln!("Jump Offset: {jump_offset}");
    jump_offset
}

pub fn jump_one_op_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    first_stack_operand: &mut i32,
    jump_offset: &mut i32,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    *jump_offset = jump_offset_helper(program_counter, method);
    // assert!(stack_pop(stack, first_stack_operand));
    *first_stack_operand = stack.pop().unwrap();
}

pub fn jump_two_ops_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    first_stack_operand: &mut i32,
    second_stack_operand: &mut i32,
    jump_offset: &mut i32,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    *jump_offset = jump_offset_helper(program_counter, method);
    // assert!(stack_pop(stack, second_stack_operand));
    // assert!(stack_pop(stack, first_stack_operand));
    *second_stack_operand = stack.pop().unwrap();
    *first_stack_operand = stack.pop().unwrap();
}

pub fn ifeq_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand == 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn ifne_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand != 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn iflt_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;

    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand < 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn ifge_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand >= 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn ifgt_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand > 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn ifle_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_one_op_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand <= 0 {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn if_icmpeq_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;

    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand == second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn if_icmpne_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand != second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn if_icmplt_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;

    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand < second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn if_icmpge_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand >= second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn if_icmpgt_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    let mut jump_offset: i32 = 0;
    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand > second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize)
    };
}

pub fn if_icmple_helper(stack: &mut Vec<i32>, program_counter: &mut usize, method: &method_t) {
    let mut jump_offset: i32 = 0;
    let mut first_stack_operand: i32 = 0;
    let mut second_stack_operand: i32 = 0;
    jump_two_ops_helper(
        stack,
        program_counter,
        method,
        &mut first_stack_operand,
        &mut second_stack_operand,
        &mut jump_offset,
    );
    if first_stack_operand <= second_stack_operand {
        *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
    };
}

pub fn goto_helper(program_counter: &mut usize, method: &method_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let jump_offset = jump_offset_helper(program_counter, method);
    *program_counter = (*program_counter).wrapping_add(jump_offset as usize);
}

pub fn ireturn_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    result: &mut Option<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    //TODO: should be able to return the option directly here vs unwrapping.
    let value = stack.pop().unwrap();
    *result = Some(value);

    // move the program counter to the end of the method so the outer while loop breaks.
    *program_counter = method.deref().code.code_length as usize;
}

pub fn return_helper(program_counter: &mut usize, method: &method_t) {
    // return by setting the program counter to the end of the instruction list,
    // thus breaking the while loop
    *program_counter = method.deref().code.code_length as usize;
}

pub fn getstatic_helper(program_counter: &mut usize) {
    // increment the program counter for the `getstatic` opcode itself.
    *program_counter = (*program_counter).wrapping_add(1);
    // increment it past the next two opcodes, for a total incrementation for each
    // use of this instruction of three, per the spec.
    *program_counter = (*program_counter).wrapping_add(TWO_OPERAND_OFFSET);
}

pub fn invokevirtual_helper(stack: &mut Vec<i32>, program_counter: &mut usize) {
    // invokevirtual b1 b2
    // Pops and prints the top value of the operand stack followed by a newline
    // character. Then, moves the program counter past b2 (i.e., increments it by
    // three).
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut value: i32 = 0;
    // assert!(stack_pop(stack, &mut value));
    let value = stack.pop().unwrap();
    println!("{value}");
    *program_counter = (*program_counter).wrapping_add(TWO_OPERAND_OFFSET);
}

pub fn not_implemented_helper(program_counter: &mut usize, opcode: &jvm_instruction_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    eprintln!("Running unimplemented opcode: {:#?}\n", *opcode);
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(false);
}

// this function needs to be always inlined into opcode_helper() and from there execute()
// or the stack will overflow on the Recursion test.
#[inline(always)]
pub fn invokestatic_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    class: &class_file_t,
    heap: &mut heap_t,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    // the instruction takes two operands from the two opcodes that follow it in the
    // code array. It then recursively executes the submethod indexed by the fusion of
    // those two operand unsigned bytes.
    let fresh13 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand = method.deref().code.code[fresh13];
    let fresh14 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: u8 = method.deref().code.code[fresh14];

    // fuse the unsigned bytes to get an index that we can use to get a pointer to the sub
    // method we want to recursively execute.
    let sub_method_index: u16 = ((first_operand as u16) << 8_i32 | second_operand as u16) as u16;
    let sub_method: Option<method_t> = find_method_from_index(sub_method_index, class);

    // double check that the sub method we got isn't NULL
    assert!(sub_method.is_some());

    // the caller of execute needs to allocate the local array using the information
    // contained in the sub method's code's max_locals variable.
    let mut locals_ptr: Vec<i32> = vec![0; sub_method.clone().unwrap().code.max_locals.into()];

    // to find out how many arguments there are to the submethod we call this helper
    // function and store the result in an unsigned short (uint16).
    let num_args: u16 = get_number_of_parameters(sub_method.as_ref().unwrap());

    // for each argument to the sub method we pop the argument's value off the stack and
    // then insert it into our sub method's locals array.
    for i in 0..num_args as usize {
        let popped_value = stack.pop().unwrap();
        locals_ptr[(num_args as usize) - (i + 1)] = popped_value;
    }

    if let Some(returned_value) = execute(&sub_method.unwrap(), &mut locals_ptr, class, heap) {
        // assert!(stack_push(stack, returned_value.unwrap()));
        stack.push(returned_value);
    }

    // locals_ptr should be dropped/freed when this returns.
}

pub fn newarray_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    heap: &mut Vec<Vec<i32>>,
) {
    // creates a new int32_t array and stores it on the heap.
    *program_counter = (*program_counter).wrapping_add(1);

    // the operand to this opcode is just the type of the array, in our cases it will
    // always be '10' to indicate its a signed 32bit integer array.
    let fresh15 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = method.code.code[fresh15];
    assert!(first_operand == 10);
    // let mut count: i32 = 0;
    // assert!(stack_pop(stack, &mut count));
    // get the size of the new array by popping the count value off the stack.
    let count: usize = stack.pop().unwrap().try_into().unwrap();
    // allocate and initialize an array of size 'count + 1', we use the first member of
    // the array to store the array's size.
    let new_array: Vec<i32> = vec![0; count];
    let tmp_ref = heap.len();
    // add the new array to the heap, and then push the index of it onto the
    // stack.
    heap.push(new_array);
    assert!(heap.len() == tmp_ref + 1);
    // assert!(stack_push(stack, tmp_ref as i32));
    stack.push(tmp_ref as i32);
}

pub fn arraylength_helper(stack: &mut Vec<i32>, program_counter: &mut usize, heap: &mut heap_t) {
    // pops a reference to an array on the heap off of the stack and then pushes that
    // array's length back on to the stack.
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));
    let reference = stack.pop().unwrap();

    let array: &Vec<i32> = &heap[reference as usize];
    // assert!(stack_push(stack, array[0]));
    // push the size of of the array back onto the stack.
    stack.push(array.len() as i32);
}

pub fn areturn_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    return_value: &mut Option<i32>,
) {
    // returns a reference to an array.
    *program_counter = (*program_counter).wrapping_add(1);
    // let mut reference: i32 = 0;
    // assert!(stack_pop(stack, &mut reference));

    // pop the reference to the array off of the stack and then set the return value to
    // it.
    let reference = stack.pop().unwrap();
    *return_value = Some(reference);

    // move the program counter to the end of the method's code to break the while loop
    // and return.
    *program_counter = method.deref().code.code_length as usize;
}

// this function needs to be always inlined into execute()
// or the stack will overflow on the Recursion test.
#[inline(always)]
fn opcode_helper(
    stack: &mut Vec<i32>,
    program_counter: &mut usize,
    method: &method_t,
    locals: &mut Vec<i32>,
    class: &class_file_t,
    heap: &mut heap_t,
    result: &mut Option<i32>,
) {
    let opcode: jvm_instruction_t =
        jvm_instruction_t::try_from(method.code.code[*program_counter]).unwrap();
    match opcode {
        jvm_instruction_t::i_nop => *program_counter += 1,
        jvm_instruction_t::i_iconst_m1
        | jvm_instruction_t::i_iconst_0
        | jvm_instruction_t::i_iconst_1
        | jvm_instruction_t::i_iconst_2
        | jvm_instruction_t::i_iconst_3
        | jvm_instruction_t::i_iconst_4
        | jvm_instruction_t::i_iconst_5 => {
            iconst_helper(opcode, stack, program_counter);
        }
        jvm_instruction_t::i_bipush => {
            bipush_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_sipush => {
            sipush_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_ldc => {
            ldc_helper(stack, program_counter, method, class);
        }
        jvm_instruction_t::i_iload => {
            iload_helper(stack, program_counter, method, locals);
        }
        jvm_instruction_t::i_aload => {
            aload_helper(stack, program_counter, method, locals);
        }
        jvm_instruction_t::i_iload_0
        | jvm_instruction_t::i_iload_1
        | jvm_instruction_t::i_iload_2
        | jvm_instruction_t::i_iload_3 => {
            iload_n_helper(opcode, stack, program_counter, locals);
        }
        jvm_instruction_t::i_aload_0
        | jvm_instruction_t::i_aload_1
        | jvm_instruction_t::i_aload_2
        | jvm_instruction_t::i_aload_3 => {
            aload_n_helper(opcode, stack, program_counter, locals);
        }
        jvm_instruction_t::i_iaload => {
            iaload_helper(stack, program_counter, heap);
        }
        jvm_instruction_t::i_istore => {
            istore_helper(stack, program_counter, method, locals);
        }
        jvm_instruction_t::i_astore => {
            astore_helper(stack, program_counter, method, locals);
        }
        jvm_instruction_t::i_istore_0
        | jvm_instruction_t::i_istore_1
        | jvm_instruction_t::i_istore_2
        | jvm_instruction_t::i_istore_3 => {
            istore_n_helper(opcode, stack, program_counter, locals);
        }
        jvm_instruction_t::i_astore_0
        | jvm_instruction_t::i_astore_1
        | jvm_instruction_t::i_astore_2
        | jvm_instruction_t::i_astore_3 => {
            astore_n_helper(opcode, stack, program_counter, locals);
        }
        jvm_instruction_t::i_iastore => {
            iastore_helper(stack, program_counter, heap);
        }
        jvm_instruction_t::i_dup => {
            dup_helper(stack, program_counter);
        }
        jvm_instruction_t::i_iadd => {
            iadd_helper(stack, program_counter);
        }
        jvm_instruction_t::i_isub => {
            isub_helper(stack, program_counter);
        }
        jvm_instruction_t::i_imul => {
            imul_helper(stack, program_counter);
        }
        jvm_instruction_t::i_idiv => {
            idiv_helper(stack, program_counter);
        }
        jvm_instruction_t::i_irem => {
            irem_helper(stack, program_counter);
        }
        jvm_instruction_t::i_ineg => {
            ineg_helper(stack, program_counter);
        }
        jvm_instruction_t::i_ishl => {
            ishl_helper(stack, program_counter);
        }
        jvm_instruction_t::i_ishr => {
            ishr_helper(stack, program_counter);
        }
        jvm_instruction_t::i_iushr => {
            iushr_helper(stack, program_counter);
        }
        jvm_instruction_t::i_iand => {
            iand_helper(stack, program_counter);
        }
        jvm_instruction_t::i_ior => {
            ior_helper(stack, program_counter);
        }
        jvm_instruction_t::i_ixor => {
            ixor_helper(stack, program_counter);
        }
        jvm_instruction_t::i_iinc => {
            iinc_helper(program_counter, method, locals);
        }
        jvm_instruction_t::i_ifeq => {
            ifeq_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_ifne => {
            ifne_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_iflt => {
            iflt_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_ifge => {
            ifge_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_ifgt => {
            ifgt_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_ifle => {
            ifle_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmpeq => {
            if_icmpeq_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmpne => {
            if_icmpne_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmplt => {
            if_icmplt_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmpge => {
            if_icmpge_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmpgt => {
            if_icmpgt_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_if_icmple => {
            if_icmple_helper(stack, program_counter, method);
        }
        jvm_instruction_t::i_goto => {
            goto_helper(program_counter, method);
        }
        jvm_instruction_t::i_ireturn => {
            ireturn_helper(stack, program_counter, method, result);
        }
        jvm_instruction_t::i_return => {
            return_helper(program_counter, method);
        }
        jvm_instruction_t::i_getstatic => {
            getstatic_helper(program_counter);
        }
        jvm_instruction_t::i_invokevirtual => {
            invokevirtual_helper(stack, program_counter);
        }
        jvm_instruction_t::i_invokestatic => {
            invokestatic_helper(stack, program_counter, method, class, heap);
        }
        jvm_instruction_t::i_newarray => {
            newarray_helper(stack, program_counter, method, heap);
        }
        jvm_instruction_t::i_arraylength => {
            arraylength_helper(stack, program_counter, heap);
        }
        jvm_instruction_t::i_areturn => {
            areturn_helper(stack, program_counter, method, result);
        }
        _ => {
            not_implemented_helper(program_counter, &opcode);
        }
    };
}

/* *
 * Runs a method's instructions until the method returns.
 *
 * @param method the method to run
 * @param locals the array of local variables, including the method parameters.
 *   Except for parameters, the locals are uninitialized.
 * @param class the class file the method belongs to
 * @param heap an array of heap-allocated pointers, useful for references
 * @return an optional int containing the method's return value
 */

pub fn execute(
    method: &method_t,
    locals: &mut Vec<i32>,
    class: &class_file_t,
    heap: &mut heap_t,
) -> Option<i32> {
    let mut program_counter: usize = 0;
    // let mut stack = stack_init(method.deref().code.max_stack.into());
    // let mut stack: Vec<i32> = vec![0; method.deref().code.max_stack.into()];
    let mut stack: Vec<i32> = Vec::with_capacity(method.deref().code.max_stack.into());
    // Return void
    let mut result: Option<i32> = None;
    while program_counter < (*method).code.code_length as usize {
        // I'm forcing this function and invokestatic to be always inlined to avoid
        // overflowing the stack in the recursion test.
        opcode_helper(
            &mut stack,
            &mut program_counter,
            method,
            locals,
            class,
            heap,
            &mut result,
        );
    }
    // stack_free(stack);
    result
}
