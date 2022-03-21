pub const NULL_FOUR_BYTES: i32 = 0x0000;
use std::cell::RefCell;
use std::ops::{Deref, DerefMut};
use std::rc::Rc;
pub fn stack_init(size: usize) -> stack_t {
    stack_t {
        contents: Vec::with_capacity(size),
        size,
        top: 0,
    }
}

pub fn stack_print(stack: &mut stack_t) {
    eprint!(
        "Printing Stack: size={}, top={}\n [",
        (*stack).size,
        (*stack).top,
    );
    let mut i: usize = 0;
    while i < (*stack).size {
        eprint!("{}, ", (*stack).contents[i]);
        i = i.wrapping_add(1)
    }
    eprintln!("]");
}

pub fn stack_pop(stack: &mut stack_t, value: &mut i32) -> bool {
    if !stack_is_empty(stack) {
        // *value = (*stack).contents[stack.top.wrapping_sub(1)];
        *value = (*stack).contents.pop().unwrap();
        // (*stack).contents[(*stack).top.wrapping_sub(1)] = NULL_FOUR_BYTES;
        (*stack).top = (*stack).top.wrapping_sub(1);
        true
    } else {
        eprintln!("Error: Stack Underflow");
        stack_print(stack);
        false
    }
}

pub fn stack_is_full(stack: &mut stack_t) -> bool {
    if stack.top >= stack.size {
        return true;
    }
    false
}

pub fn stack_is_empty(stack: &stack_t) -> bool {
    stack.top == 0
}

pub fn stack_push(stack: &mut stack_t, value: i32) -> bool {
    // This can all be replaced with the Vec
    if !stack_is_full(stack) {
        // stack.contents[stack.top] = value;
        stack.contents.push(value);
        stack.top = stack.top.wrapping_add(1);
        true
    } else {
        eprintln!("Error: Stack Overflow, value: {}", value,);
        stack_print(stack);
        false
    }
}

#[derive(Clone, Debug)]
#[repr(C)]
pub struct stack_t {
    pub size: usize,
    pub top: usize,
    pub contents: Vec<i32>,
}
