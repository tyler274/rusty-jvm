#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "jvm.h"
#include "read_class.h"

/** The name of the method to invoke to run the class file */
const char *MAIN_METHOD = "main";
/**
 * The "descriptor" string for main(). The descriptor encodes main()'s signature,
 * i.e. main() takes a String[] and returns void.
 * If you're interested, the descriptor string is explained at
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-4.html#jvms-4.3.2.
 */
const char *MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";

/**
 * Runs a method's instructions until the method returns.
 *
 * @param method the method to run
 * @param locals the array of local variables, including the method parameters.
 *   Except for parameters, the locals are uninitialized.
 * @param class the class file the method belongs to
 * @return if the method returns an int, a heap-allocated pointer to it;
 *   if the method returns void, NULL
 */
int32_t *execute(method_t *method, int32_t *locals, class_file_t *class) {
    /* You should remove these casts to void in your solution.
     * They are just here so the code compiles without warnings. */
    (void) method;
    (void) locals;
    (void) class;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <class file>\n", argv[0]);
        return 1;
    }

    // Open the class file for reading
    FILE *class_file = fopen(argv[1], "r");
    assert(class_file && "Failed to open file");

    // Parse the class file
    class_file_t class = get_class(class_file);
    int error = fclose(class_file);
    assert(!error && "Failed to close file");

    // Execute the main method
    method_t *main_method = find_method(MAIN_METHOD, MAIN_DESCRIPTOR, &class);
    assert(main_method && "Missing main() method");
    /* In a real JVM, locals[0] would contain a reference to String[] args.
     * But since TeenyJVM doesn't support Objects, we leave it uninitialized. */
    int32_t locals[main_method->code.max_locals];
    int32_t *result = execute(main_method, locals, &class);
    assert(!result && "main() should return void");

    // Free the internal data structures
    free_class(&class);
}
