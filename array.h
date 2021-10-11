#ifndef ARRAY_H
#define ARRAY_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t size;
    size_t position;
    int32_t *contents;
} array_t;

// init a stack struct in memory and return a pointer to it
array_t *array_init() {
    // The stack  array is initially allocated to hold zero elements.
    array_t *array = (array_t *) calloc(1, sizeof(array_t));
    // atm we use the heap to manage memory for stacks
    array->contents = NULL;
    array->size = 0;
    array->position = 0;
    return array;
}

void array_free(array_t *array) {
    /**
     *  At the moment the stack's contents are allocated on the heap, and to avoid double
     * free UB should be freed there as well.
     */
    free(array->contents);
    free(array);
}

void array_print(array_t *array) {
    fprintf(stderr, "Printing array: size=%zu, position=%zu\n [", array->size,
            array->position);
    for (size_t i = 0; i < array->size; i++) {
        fprintf(stderr, "%d, ", array->contents[i]);
    }
    fprintf(stderr, "]\n");
}

#endif /* ARRAY_H */