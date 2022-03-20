use std::cell::RefCell;

use std::fs::File;
use std::io::prelude::*;
use std::io::{BufRead, BufReader, Seek, SeekFrom};
use std::rc::Rc;

#[derive(Copy, Clone, Debug, PartialEq)]
#[repr(C)]
pub struct class_header_t {
    pub magic: u32,
    pub minor_version: u16,
    pub major_version: u16,
}
#[derive(Copy, Clone, Debug, PartialEq)]
#[repr(C)]
pub struct class_info_t {
    pub access_flags: u16,
    pub this_class: u16,
    pub super_class: u16,
}
#[derive(Copy, Clone, Debug, PartialEq)]
#[repr(C)]
pub struct method_info {
    pub access_flags: u16,
    pub name_index: u16,
    pub descriptor_index: u16,
    pub attributes_count: u16,
}
#[derive(Copy, Clone, Debug, PartialEq)]
#[repr(C)]
pub struct attribute_info {
    pub attribute_name_index: u16,
    pub attribute_length: u32,
}
#[derive(Clone, Debug, PartialEq)]
#[repr(C)]
pub struct code_t {
    pub max_stack: u16,
    pub max_locals: u16,
    pub code_length: u32,
    pub code: Vec<u8>,
}
#[derive(Clone, Debug)]
#[repr(C)]
pub struct method_t {
    pub name: String,
    pub descriptor: cp_info_t,
    pub code: code_t,
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum cp_info_tag {
    CONSTANT_NameAndType = 12,
    CONSTANT_Methodref = 10,
    CONSTANT_Fieldref = 9,
    CONSTANT_Class = 7,
    CONSTANT_Integer = 3,
    CONSTANT_Utf8 = 1,
}

impl std::convert::TryFrom<u8> for cp_info_tag {
    type Error = ();

    fn try_from(v: u8) -> Result<Self, Self::Error> {
        match v {
            1 => Ok(cp_info_tag::CONSTANT_Utf8),
            3 => Ok(cp_info_tag::CONSTANT_Integer),
            7 => Ok(cp_info_tag::CONSTANT_Class),
            9 => Ok(cp_info_tag::CONSTANT_Fieldref),
            10 => Ok(cp_info_tag::CONSTANT_Methodref),
            12 => Ok(cp_info_tag::CONSTANT_NameAndType),
            _ => Err(()),
        }
    }
}

impl std::convert::TryFrom<i32> for cp_info_tag {
    type Error = ();

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            1 => Ok(cp_info_tag::CONSTANT_Utf8),
            3 => Ok(cp_info_tag::CONSTANT_Integer),
            7 => Ok(cp_info_tag::CONSTANT_Class),
            9 => Ok(cp_info_tag::CONSTANT_Fieldref),
            10 => Ok(cp_info_tag::CONSTANT_Methodref),
            12 => Ok(cp_info_tag::CONSTANT_NameAndType),
            _ => Err(()),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum cp_info_t {
    CONSTANT_Integer_info {
        bytes: i32,
    },
    CONSTANT_Class_info {
        string_index: u16,
    },
    CONSTANT_FieldOrMethodref_info {
        class_index: u16,
        name_and_type_index: u16,
    },
    CONSTANT_NameAndType_info {
        name_index: u16,
        descriptor_index: u16,
    },
    CONSTANT_Utf8_info {
        descriptor: String,
    },
}

#[derive(Clone, Debug, PartialEq)]
#[repr(C)]
pub struct cp_info {
    pub tag: cp_info_tag,
    pub info: Option<Box<cp_info_t>>,
}
#[derive(Clone, Debug)]
#[repr(C)]
pub struct class_file_t {
    pub constant_pool: Vec<cp_info>,
    pub methods: Vec<method_t>,
}

pub const CLASS_MAGIC: u32 = 0xcafebabe;

pub const IS_STATIC: u16 = 0x8;

pub type ClassFile = Rc<RefCell<BufReader<File>>>;

/*
 * Functions for reading unsigned big-endian integers. We can't read directly
 * into a u16 or u32 variable because x86 stores integers in little-endian.
 */

pub fn read_u8(class_file: ClassFile) -> u8 {
    let mut value: [u8; 1] = [0];
    class_file.borrow_mut().read_exact(&mut value).unwrap();
    u8::from_be_bytes(value)
}

pub fn read_u16(class_file: ClassFile) -> u16 {
    let mut value: [u8; 2] = [0; 2];
    class_file.borrow_mut().read_exact(&mut value).unwrap();
    u16::from_be_bytes(value)
}

pub fn read_u32(class_file: ClassFile) -> u32 {
    let mut value: [u8; 4] = [0; 4];
    class_file.borrow_mut().read_exact(&mut value).unwrap();
    u32::from_be_bytes(value)
}

pub fn read_bytes(class_file: ClassFile, length: usize) -> Vec<u8> {
    let mut value = vec![0_u8; length];
    class_file.borrow_mut().read_exact(&mut value).unwrap();
    value
}

pub fn constant_pool_size(constant_pool: Vec<cp_info>) -> u16 {
    let mut constant = constant_pool[0].clone();
    let mut i: usize = 0;
    while constant.info.is_some() {
        i += 1;
        constant = constant_pool[i].clone()
    }
    i as u16
}

pub fn get_constant(constant_pool: Vec<cp_info>, index: u16) -> cp_info {
    let const_size = constant_pool_size(constant_pool.clone());
    if 0 < index && index <= const_size {
    } else {
        eprintln!("Invalid constant pool index");
    }
    // Convert 1-indexed index to 0-indexed index
    constant_pool[((index as i32) - 1) as usize].clone()
}

pub fn get_method_name_and_type(constant_pool: Vec<cp_info>, index: u16) -> Option<Box<cp_info_t>> {
    let method_constant: cp_info = get_constant(constant_pool.clone(), index);
    if method_constant.tag as u32 == cp_info_tag::CONSTANT_Methodref as u32 {
    } else {
        eprintln!("Expected a MethodRef");
        std::process::exit(1);
    }

    match method_constant.info.as_deref() {
        Some(cp_info_t::CONSTANT_FieldOrMethodref_info {
            class_index: _,
            name_and_type_index,
        }) => {
            let name_and_type_constant: cp_info = get_constant(constant_pool, *name_and_type_index);

            if name_and_type_constant.tag as u32 == cp_info_tag::CONSTANT_NameAndType as i32 as u32
            {
            } else {
                eprintln!("Expected a NameAndType");
                std::process::exit(1);
            }
            match (name_and_type_constant).info.as_deref() {
                Some(cp_info_t::CONSTANT_NameAndType_info { .. }) => name_and_type_constant.info,
                _ => {
                    eprintln!("Expected a NameAndType");
                    std::process::exit(1);
                }
            }
        }
        _ => {
            eprintln!("Expected a MethodRef");
            std::process::exit(1);
        }
    }
    // return None;
}

pub fn get_number_of_parameters(method: &method_t) -> u16 {
    // Type descriptors will always have the length ( + #params + ) + return type
    let end: usize;
    let mut start: usize;
    match &(*method).descriptor {
        cp_info_t::CONSTANT_Utf8_info { descriptor: d } => {
            end = d.find(')').unwrap();
            start = d.find('(').unwrap();
            let mut params: u16 = 0;
            start += 1;
            while start < end {
                if d.get(start..start) == Some("[") {
                    start += 1;
                }
                params = params.wrapping_add(1);
                start += 1;
            }
            params
        }
        _ => {
            eprintln!("Method descriptor should be a utf8 string");
            std::process::exit(1);
        }
    }
}

pub fn find_method(name: &str, descriptor: &str, class: &class_file_t) -> Option<method_t> {
    let mut i: usize = 0;
    let mut method: &method_t = &(*class).methods[i];
    while !(*method).name.is_empty() {
        if (*method).name == name
            && descriptor
                == match &(*method).descriptor {
                    cp_info_t::CONSTANT_Utf8_info { descriptor: d } => d,
                    _ => {
                        eprintln!("Descriptor of a method should be a utf8 string");
                        std::process::exit(1);
                    }
                }
        {
            return Some(method.clone());
        }
        i += 1;
        method = &(*class).methods[i];
    }
    None
}

pub fn find_method_from_index(index: u16, class: &class_file_t) -> Option<method_t> {
    let (name_index, descriptor_index) =
        match get_method_name_and_type((*class).constant_pool.clone(), index).as_deref() {
            Some(cp_info_t::CONSTANT_NameAndType_info {
                name_index: n,
                descriptor_index: d,
            }) => (*n, *d),
            _ => {
                eprintln!("Expected a name and type");
                std::process::exit(1);
            }
        };

    let name: cp_info = get_constant((*class).constant_pool.clone(), name_index);
    if name.tag as u32 != cp_info_tag::CONSTANT_Utf8 as i32 as u32 {
        eprintln!("Expected a UTF8");
        std::process::exit(1);
    }
    let descriptor: cp_info = get_constant(class.constant_pool.clone(), descriptor_index);
    if descriptor.tag as u32 != cp_info_tag::CONSTANT_Utf8 as i32 as u32 {
        eprintln!("Expected a UTF8");
        std::process::exit(1);
    }

    let (n, d) = match (*name.info.unwrap(), *descriptor.info.unwrap()) {
        (
            cp_info_t::CONSTANT_Utf8_info { descriptor: nam },
            cp_info_t::CONSTANT_Utf8_info { descriptor: des },
        ) => (nam, des),
        _ => {
            eprintln!("Expected a UTF8");
            std::process::exit(1);
        }
    };
    find_method(&n, &d, class)
}

pub fn get_class_header(class_file: ClassFile) -> class_header_t {
    let mut header: class_header_t = class_header_t {
        magic: 0,
        minor_version: 0,
        major_version: 0,
    };
    header.magic = read_u32(class_file.clone());
    assert!(header.magic == CLASS_MAGIC);
    header.major_version = read_u16(class_file.clone());
    header.minor_version = read_u16(class_file);
    header
}

pub fn get_constant_pool(class_file: ClassFile) -> Vec<cp_info> {
    // Constant pool count includes unused constant at index 0
    let mut constant_pool_count: u16 = (read_u16(class_file.clone()) as i32 - 1) as u16;
    let mut constant_pool: Vec<cp_info> = Vec::with_capacity(constant_pool_count as usize + 1);

    while constant_pool_count > 0 {
        let mut constant: cp_info = cp_info {
            tag: cp_info_tag::CONSTANT_Utf8,
            info: None,
        };
        constant.tag = cp_info_tag::try_from(read_u8(class_file.clone())).unwrap();
        match constant.tag {
            cp_info_tag::CONSTANT_Utf8 => {
                let length: u16 = read_u16(class_file.clone());
                let mut info =
                    String::from_utf8(read_bytes(class_file.clone(), length as usize)).unwrap();
                info.push('\u{0}');
                let info_struct = Box::new(cp_info_t::CONSTANT_Utf8_info { descriptor: info });
                constant.info = Some(info_struct)
            }
            cp_info_tag::CONSTANT_Integer => {
                let value: Box<cp_info_t> = Box::new(cp_info_t::CONSTANT_Integer_info {
                    bytes: read_u32(class_file.clone()) as i32,
                });
                constant.info = Some(value);
            }
            cp_info_tag::CONSTANT_Class => {
                let value_0: Box<cp_info_t> = Box::new(cp_info_t::CONSTANT_Class_info {
                    string_index: read_u16(class_file.clone()),
                });
                constant.info = Some(value_0);
            }
            cp_info_tag::CONSTANT_Fieldref | cp_info_tag::CONSTANT_Methodref => {
                let value_1: Box<cp_info_t> = Box::new(cp_info_t::CONSTANT_FieldOrMethodref_info {
                    class_index: read_u16(class_file.clone()),
                    name_and_type_index: read_u16(class_file.clone()),
                });
                constant.info = Some(value_1);
            }
            cp_info_tag::CONSTANT_NameAndType => {
                let value_2: Box<cp_info_t> = Box::new(cp_info_t::CONSTANT_NameAndType_info {
                    name_index: read_u16(class_file.clone()),
                    descriptor_index: read_u16(class_file.clone()),
                });
                constant.info = Some(value_2);
            }
            _ => {
                eprintln!("Unknown constant type {:#?}", constant.tag);
                std::process::exit(1);
            }
        }
        // i += 1;
        constant_pool.push(constant.clone());

        constant_pool_count = constant_pool_count.wrapping_sub(1)
    }
    // Mark end of array with NULL info
    constant_pool.push(cp_info {
        tag: cp_info_tag::CONSTANT_Integer,
        info: None,
    });
    constant_pool
}

pub fn get_class_info(class_file: ClassFile) -> class_info_t {
    let mut info: class_info_t = class_info_t {
        access_flags: 0,
        this_class: 0,
        super_class: 0,
    };
    info.access_flags = read_u16(class_file.clone());
    info.this_class = read_u16(class_file.clone());
    info.super_class = read_u16(class_file.clone());
    let interfaces_count: u16 = read_u16(class_file.clone());
    if interfaces_count != 0 {
        eprintln!("This VM does not support interfaces.");
        std::process::exit(1);
    }

    let fields_count: u16 = read_u16(class_file);
    if fields_count != 0 {
        eprintln!("This VM does not support fields.");
        std::process::exit(1);
    }
    info
}

pub fn read_method_attributes(
    class_file: ClassFile,
    info: &method_info,
    code: &mut code_t,
    constant_pool: Vec<cp_info>,
) {
    let mut found_code: bool = false;
    let mut attributes: u16 = info.attributes_count;
    while attributes > 0 {
        let mut ainfo: attribute_info = attribute_info {
            attribute_name_index: 0,
            attribute_length: 0,
        };
        ainfo.attribute_name_index = read_u16(class_file.clone());
        ainfo.attribute_length = read_u32(class_file.clone());
        let attribute_end =
            class_file.borrow_mut().stream_position().unwrap() + ainfo.attribute_length as u64;
        let type_constant: cp_info =
            get_constant(constant_pool.clone(), ainfo.attribute_name_index);
        if type_constant.tag != cp_info_tag::CONSTANT_Utf8 {
            eprintln!("Expected a UTF8");
            std::process::exit(1);
        }
        let type_constant_info = match type_constant.info.as_deref() {
            Some(cp_info_t::CONSTANT_Utf8_info { descriptor }) => descriptor,
            _ => "",
        };
        println!("{}", type_constant_info);
        if *type_constant_info == *"Code" {
            if found_code {
                eprintln!("Duplicate method code");
                std::process::exit(1);
            }

            found_code = true;
            code.max_stack = read_u16(class_file.clone());
            code.max_locals = read_u16(class_file.clone());
            code.code_length = read_u32(class_file.clone());
            // code.code = Vec::with_capacity(code.code_length as usize);

            code.code = read_bytes(class_file.clone(), code.code_length as usize);
        }

        // Skip the rest of the attribute
        class_file
            .borrow_mut()
            .seek(SeekFrom::Start(attribute_end as u64))
            .unwrap();
        attributes = attributes.wrapping_sub(1)
    }
    if !found_code {
        eprintln!("Missing method code");
        std::process::exit(1);
    }
}

pub fn get_methods(class_file: ClassFile, constant_pool: &Vec<cp_info>) -> Vec<method_t> {
    let mut method_count: u16 = read_u16(class_file.clone());
    let mut methods: Vec<method_t> = Vec::with_capacity((method_count + 1).into());
    // let mut i: usize = 0;
    // let mut method: &mut method_t = &mut methods[i];
    while method_count > 0 {
        let mut method = method_t {
            name: String::new(),
            descriptor: cp_info_t::CONSTANT_Utf8_info {
                descriptor: String::new(),
            },
            code: code_t {
                max_stack: 0,
                max_locals: 0,
                code_length: 0,
                code: Vec::new(),
            },
        };
        let mut info: method_info = method_info {
            access_flags: 0,
            name_index: 0,
            descriptor_index: 0,
            attributes_count: 0,
        };
        info.access_flags = read_u16(class_file.clone());
        info.name_index = read_u16(class_file.clone());
        info.descriptor_index = read_u16(class_file.clone());
        info.attributes_count = read_u16(class_file.clone());
        let name: cp_info = get_constant(constant_pool.clone(), info.name_index);
        if name.tag as u32 != cp_info_tag::CONSTANT_Utf8 as i32 as u32 {
            eprintln!("Expected a UTF8");
            std::process::exit(1);
        }
        method.name = match *name.info.unwrap() {
            cp_info_t::CONSTANT_Utf8_info { descriptor } => descriptor,
            _ => {
                eprintln!("Expected a UTF8");
                std::process::exit(1);
            }
        };
        let descriptor: cp_info = get_constant(constant_pool.clone(), info.descriptor_index);
        if descriptor.tag as u32 != cp_info_tag::CONSTANT_Utf8 as i32 as u32 {
            eprintln!("Expected a UTF8");
            std::process::exit(1);
        }
        method.descriptor = *descriptor.info.unwrap();
        /* Our JVM can only execute static methods, so ensure all methods are static.
         * However, javac creates a constructor method <init> we need to ignore. */
        if method.name != "<init>" {
            if (info.access_flags & IS_STATIC) == 0 {
                eprintln!("This VM only supports static methods.");
                std::process::exit(1);
            }
        }
        read_method_attributes(
            class_file.clone(),
            &info,
            &mut method.code,
            constant_pool.clone(),
        );
        methods.push(method.clone());
        method_count = method_count.wrapping_sub(1)
    }
    // Mark end of array with NULL name
    // method.name = "".to_string();
    methods.push(method_t {
        name: String::new(),
        descriptor: cp_info_t::CONSTANT_Utf8_info {
            descriptor: String::new(),
        },
        code: code_t {
            max_stack: 0,
            max_locals: 0,
            code_length: 0,
            code: Vec::new(),
        },
    });
    methods
}

pub fn get_class(class_file: ClassFile) -> Rc<RefCell<class_file_t>> {
    let class: Rc<RefCell<class_file_t>> = Rc::new(RefCell::new(class_file_t {
        constant_pool: Vec::new(),
        methods: Vec::new(),
    }));
    /* Read the leading header of the class file.
     * We don't need the result, but we need to skip past the header. */
    get_class_header(class_file.clone());
    // Read the constant pool
    class.borrow_mut().constant_pool = get_constant_pool(class_file.clone());
    /* Read information about the class that was compiled.
     * We don't need the result, but we need to skip past it. */
    get_class_info(class_file.clone());
    // Read the list of static methods
    let meth_vec: Vec<method_t> = get_methods(class_file, &class.borrow().constant_pool.clone());
    class.borrow_mut().methods = meth_vec;
    class
}
