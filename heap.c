#include "heap.h"

#include <stdlib.h>

heap_t *heap_init() {
    // The heap array is initially allocated to hold zero elements.
    heap_t *heap = malloc(sizeof(heap_t));
    heap->ptr = malloc(0);
    heap->count = 0;
    return heap;
}

void free_heap(heap_t *heap) {
    for (size_t i = 0; i < heap->count; i++) {
        free(heap->ptr[i]);
    }
    free(heap->ptr);
    free(heap);
}