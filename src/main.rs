mod jvm;
mod opcode;
mod read_class;
mod stack;

use jvm::main_0;

pub fn main() {
    std::process::exit(main_0())
}
