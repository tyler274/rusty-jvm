use crate::opcode::jvm_instruction_t;
use crate::read_class::{
    class_file_t, code_t, cp_info, cp_info_t, cp_info_tag, find_method, find_method_from_index,
    get_class, get_number_of_parameters, method_t,
};
use crate::stack::{
    stack_init, stack_is_empty, stack_is_full, stack_pop, stack_print, stack_push, stack_t,
};

use std::cell::RefCell;
use std::fs::File;
use std::io::prelude::*;
use std::io::BufReader;
use std::ops::Deref;
use std::process::exit;
use std::rc::Rc;

pub type heap_t = Vec<Vec<i32>>;

pub const TWO_OPERAND_OFFSET: usize = 2;

// pub const NEGATIVE_ONE: i32 = -1;

// pub const ZERO: i32 = 0;

// pub const ONE: i32 = 1;

// pub const TWO: i32 = 2;

pub const THREE: i32 = 3;

// pub const FOUR: i32 = 4;

// pub const FIVE: i32 = 5;

pub fn iconst_helper(opcode: jvm_instruction_t, stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(stack_push(stack, (opcode as i32) - THREE));
}

pub fn bipush_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh0 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(stack_push(stack, (*method).code.code[fresh0] as i8 as i32,));
}

pub fn sipush_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh1 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh1 as usize];
    let fresh2 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: u8 = (*method).code.code[fresh2];
    let result: i16 = ((first_operand as i16 as i32) << 8_i32 | second_operand as i32) as i16;
    assert!(stack_push(stack, result as i32));
}

pub fn constant_pool_helper(stack: &mut stack_t, pool_const: &cp_info) {
    assert!(pool_const.info.is_some());
    match pool_const.tag {
        cp_info_tag::CONSTANT_Integer => match pool_const.info.as_deref() {
            Some(cp_info_t::CONSTANT_Integer_info { bytes }) => {
                assert!(stack_push(stack, *bytes as i32))
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
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    class: &class_file_t,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh3 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let pool_index = (*method).code.code[fresh3] as usize;

    let pool_const: cp_info = (*class).constant_pool[pool_index.wrapping_sub(1) as usize].clone();
    if (*class).constant_pool[pool_index.wrapping_sub(1) as usize]
        .info
        .is_some()
    {
        constant_pool_helper(stack, &pool_const);
    };
}

pub fn iload_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh4 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh4];
    assert!(stack_push(stack, locals[first_operand as usize]));
}

pub fn aload_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh5 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(stack_push(
        stack,
        locals[(*method).code.code[fresh5] as usize],
    ));
}

pub fn iload_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut stack_t,
    program_counter: &mut usize,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(stack_push(
        stack,
        locals[(opcode as usize).wrapping_sub(jvm_instruction_t::i_iload_0 as usize)]
    ));
}

pub fn aload_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut stack_t,
    program_counter: &mut usize,
    locals: &[i32],
) {
    *program_counter = (*program_counter).wrapping_add(1);
    assert!(stack_push(
        stack,
        locals[(opcode as usize).wrapping_sub(jvm_instruction_t::i_aload_0 as usize) as usize]
    ));
}

pub fn iaload_helper(stack: &mut stack_t, program_counter: &mut usize, heap: &mut heap_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut index: i32 = 0;
    assert!(stack_pop(stack, &mut index));
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));
    let array: Vec<i32> = heap[reference as usize].clone();
    assert!((index as usize) < array.len());
    let value: i32 = array[index as usize];
    assert!(stack_push(stack, value));
}

pub fn istore_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh6 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh6];
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));
    locals[first_operand as usize] = value;
}

pub fn astore_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh7 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh7];
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));
    locals[first_operand as usize] = reference;
}

pub fn istore_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut stack_t,
    program_counter: &mut usize,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let locals_index: usize =
        (opcode as usize).wrapping_sub(jvm_instruction_t::i_istore_0 as usize);
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));
    locals[locals_index] = value;
}

pub fn astore_n_helper(
    opcode: jvm_instruction_t,
    stack: &mut stack_t,
    program_counter: &mut usize,
    locals: &mut Vec<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let locals_index: usize =
        (opcode as usize).wrapping_sub(jvm_instruction_t::i_astore_0 as usize);
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));
    locals[locals_index] = reference;
}

pub fn iastore_helper(stack: &mut stack_t, program_counter: &mut usize, heap: &mut heap_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));
    let mut index: i32 = 0;
    assert!(stack_pop(stack, &mut index));
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));
    let array: &mut Vec<i32> = &mut heap[reference as usize];
    assert!((index as usize) < array.len());
    array[index as usize] = value;
}

pub fn dup_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));
    assert!(stack_push(stack, value));
    assert!(stack_push(stack, value));
}

pub fn iadd_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(
        stack,
        first_operand.wrapping_add(second_operand)
    ));
}

pub fn isub_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(
        stack,
        first_operand.wrapping_sub(second_operand)
    ));
}

pub fn imul_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(
        stack,
        first_operand.wrapping_mul(second_operand)
    ));
}

pub fn idiv_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    // eprintln!("idiv First Operand: {first_operand}, Second Operand: {second_operand}");
    assert!(stack_push(stack, second_operand / first_operand));
}

pub fn irem_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, second_operand % first_operand));
}

pub fn ineg_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand.wrapping_neg()));
}

pub fn ishl_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand << second_operand));
}

pub fn ishr_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand >> second_operand));
}

pub fn iushr_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(
        stack,
        (first_operand as u32 >> second_operand) as i32,
    ));
}

pub fn iand_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand & second_operand));
}

pub fn ior_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand | second_operand));
}

pub fn ixor_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut first_operand: i32 = 0;
    let mut second_operand: i32 = 0;
    assert!(stack_pop(stack, &mut second_operand));
    assert!(stack_pop(stack, &mut first_operand));
    assert!(stack_push(stack, first_operand ^ second_operand));
}

pub fn iinc_helper(program_counter: &mut usize, method: &method_t, locals: &mut Vec<i32>) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh8 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = (*method).code.code[fresh8];
    let fresh9 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: i32 = (*method).code.code[fresh9] as i32;
    locals[first_operand as usize] += second_operand;
}

pub const JUMP_TWO_OPCODES_OFFSET: i32 = -3;

pub fn jump_offset_helper(program_counter: &mut usize, method: &method_t) -> i32 {
    let fresh11 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: i8 = (*method).code.code[fresh11] as i8;
    let fresh12 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: i8 = (*method).code.code[fresh12] as i8;
    let jump_offset: i32 = (((first_operand as i16 as i32) << 8_i32) as i16 as i32
        | second_operand as i32)
        + JUMP_TWO_OPCODES_OFFSET;
    jump_offset
}

pub fn jump_one_op_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    first_stack_operand: &mut i32,
    jump_offset: &mut i32,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    *jump_offset = jump_offset_helper(program_counter, method);
    assert!(stack_pop(stack, first_stack_operand));
}

pub fn jump_two_ops_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    first_stack_operand: &mut i32,
    second_stack_operand: &mut i32,
    jump_offset: &mut i32,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    *jump_offset = jump_offset_helper(program_counter, method);
    assert!(stack_pop(stack, second_stack_operand));
    assert!(stack_pop(stack, first_stack_operand));
}

pub fn ifeq_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn ifne_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn iflt_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn ifge_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn ifgt_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn ifle_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmpeq_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmpne_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmplt_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmpge_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmpgt_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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

pub fn if_icmple_helper(stack: &mut stack_t, program_counter: &mut usize, method: &method_t) {
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
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    result: &mut Option<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));
    *result = Some(value);

    *program_counter = method.code.code_length as usize;
}

pub fn return_helper(program_counter: &mut usize, method: &method_t) {
    *program_counter = (*method).code.code_length as usize;
}

pub fn invokevirtual_helper(stack: &mut stack_t, program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut value: i32 = 0;
    assert!(stack_pop(stack, &mut value));

    println!("{}", value,);
    *program_counter = (*program_counter).wrapping_add(TWO_OPERAND_OFFSET);
}

#[inline(always)]
pub fn invokestatic_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    class: &class_file_t,
    heap: &mut heap_t,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh13 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand = (*method).code.code[fresh13];
    let fresh14 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let second_operand: u8 = (*method).code.code[fresh14];
    let sub_method_index: u16 = ((first_operand as i32) << 8_i32 | second_operand as i32) as u16;
    let sub_method: Option<method_t> = find_method_from_index(sub_method_index, class);
    assert!(sub_method.is_some());
    let mut locals_ptr: Vec<i32> =
        Vec::with_capacity(sub_method.clone().unwrap().code.max_locals.into());

    let num_args: usize = get_number_of_parameters(sub_method.as_ref().unwrap()) as usize;
    let mut popped_value: i32 = 0;
    let mut i: usize = 0;
    while i < num_args {
        assert!(stack_pop(stack, &mut popped_value));

        locals_ptr.push(popped_value);
        i = i.wrapping_add(1)
    }
    let returned_value: Option<i32> = execute(&sub_method.unwrap(), &mut locals_ptr, class, heap);
    if returned_value.is_some() {
        assert!(stack_push(stack, returned_value.unwrap()));
    }
}

pub fn newarray_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    heap: &mut Vec<Vec<i32>>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let fresh15 = *program_counter;
    *program_counter = (*program_counter).wrapping_add(1);
    let first_operand: u8 = method.code.code[fresh15];
    assert!(first_operand == 10);
    let mut count: i32 = 0;
    assert!(stack_pop(stack, &mut count));
    let new_array: Vec<i32> = Vec::with_capacity(count.try_into().unwrap());
    let tmp_ref = heap.len();
    heap.push(new_array);
    assert!(stack_push(stack, tmp_ref as i32));
}

pub fn arraylength_helper(stack: &mut stack_t, program_counter: &mut usize, heap: &mut heap_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));

    let array: Vec<i32> = heap[reference as usize].clone();
    assert!(stack_push(stack, array[0]));
}

pub fn getstatic_helper(program_counter: &mut usize) {
    *program_counter = (*program_counter).wrapping_add(1);
    *program_counter = (*program_counter).wrapping_add(TWO_OPERAND_OFFSET);
}

pub fn areturn_helper(
    stack: &mut stack_t,
    program_counter: &mut usize,
    method: &method_t,
    return_value: &mut Option<i32>,
) {
    *program_counter = (*program_counter).wrapping_add(1);
    let mut reference: i32 = 0;
    assert!(stack_pop(stack, &mut reference));
    *return_value = Some(reference);
    *program_counter = (*method).code.code_length as usize;
}

pub fn not_implemented_helper(program_counter: &mut usize, opcode: &jvm_instruction_t) {
    *program_counter = (*program_counter).wrapping_add(1);
    eprintln!("Running unimplemented opcode: {:#?}\n", *opcode);
    *program_counter = (*program_counter).wrapping_add(1);
}
#[inline(always)]
fn opcode_helper(
    stack: &mut stack_t,
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

/* * The name of the method to invoke to run the class file */
pub const MAIN_METHOD: &str = "main";
/* *
 * The "descriptor" string for main(). The descriptor encodes main()'s signature,
 * i.e. main() takes a String[] and returns void.
 * If you're interested, the descriptor string is explained at
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-4.html#jvms-4.3.2.
 */
pub const MAIN_DESCRIPTOR: &str = "([Ljava/lang/String;)V";
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
    let mut stack = stack_init(method.deref().code.max_stack.into());
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
pub fn main_0() -> i32 {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("USAGE: {} <class file>", args.len(),);
        return 1;
    }
    // Open the class file for reading
    let f = File::open(args[1].clone()).unwrap();
    let class_file = Rc::new(RefCell::new(BufReader::new(f)));

    // Parse the class file
    let class: Rc<RefCell<class_file_t>> = get_class(class_file);

    // The heap array is initially allocated to hold zero elements.
    let mut heap: Vec<Vec<i32>> = Vec::new();
    // Execute the main method
    let main_method: Option<method_t> = find_method(MAIN_METHOD, MAIN_DESCRIPTOR, &class.borrow());
    if main_method.is_none() {
        eprintln!("Missing main() method");
        exit(1);
    }
    /* In a real JVM, locals[0] would contain a reference to String[] args.
     * But since TeenyJVM doesn't support Objects, we leave it uninitialized. */
    let mut locals: Vec<i32> =
        std::vec::from_elem(0, main_method.as_ref().unwrap().code.max_locals.into());
    let result: Option<i32> = execute(
        main_method.as_ref().unwrap(),
        &mut locals,
        &class.borrow(),
        &mut heap,
    );
    if result.is_some() {
        eprintln!("main() should return void");
        exit(1);
    }
    0
}
