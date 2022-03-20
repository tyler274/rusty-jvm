#![allow(dead_code, mutable_transmutes, non_camel_case_types, non_snake_case,
         non_upper_case_globals, unused_assignments, unused_mut)]
#![register_tool(c2rust)]
#![feature(register_tool)]
extern "C" {
    #[no_mangle]
    fn realloc(_: *mut libc::c_void, _: libc::c_ulong) -> *mut libc::c_void;
    #[no_mangle]
    fn malloc(_: libc::c_ulong) -> *mut libc::c_void;
    #[no_mangle]
    fn free(__ptr: *mut libc::c_void);
}
pub type __int32_t = libc::c_int;
pub type int32_t = __int32_t;
#[derive(Copy, Clone)]
#[repr(C)]
pub struct heap {
    pub ptr: *mut *mut int32_t,
    pub count: int32_t,
}
pub type heap_t = heap;
#[no_mangle]
pub unsafe extern "C" fn heap_init() -> *mut heap_t {
    // The heap array is initially allocated to hold zero elements.
    let mut heap: *mut heap_t =
        malloc(::std::mem::size_of::<heap_t>() as libc::c_ulong) as
            *mut heap_t;
    (*heap).ptr = 0 as *mut *mut int32_t;
    (*heap).count = 0 as libc::c_int;
    return heap;
}
#[no_mangle]
pub unsafe extern "C" fn heap_add(mut heap: *mut heap_t,
                                  mut ptr: *mut int32_t) -> int32_t {
    (*heap).ptr =
        realloc((*heap).ptr as *mut libc::c_void,
                (((*heap).count + 1 as libc::c_int) as
                     libc::c_ulong).wrapping_mul(::std::mem::size_of::<*mut int32_t>()
                                                     as libc::c_ulong)) as
            *mut *mut int32_t;
    let ref mut fresh0 = *(*heap).ptr.offset((*heap).count as isize);
    *fresh0 = ptr;
    let mut temp: int32_t = (*heap).count;
    (*heap).count += 1 as libc::c_int;
    return temp;
}
#[no_mangle]
pub unsafe extern "C" fn heap_get(mut heap: *mut heap_t, mut ref_0: int32_t)
 -> *mut int32_t {
    return *(*heap).ptr.offset(ref_0 as isize);
}
#[no_mangle]
pub unsafe extern "C" fn heap_free(mut heap: *mut heap_t) {
    let mut i: int32_t = 0 as libc::c_int;
    while i < (*heap).count {
        free(*(*heap).ptr.offset(i as isize) as *mut libc::c_void);
        i += 1
    }
    free((*heap).ptr as *mut libc::c_void);
    free(heap as *mut libc::c_void);
}
