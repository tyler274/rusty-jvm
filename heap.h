/**
 * Represents the array of heap-allocated pointers.
 */
typedef struct {
    /** Generic array of pointers. */
    void **ptr;
    /** How many pointers there are currently in the array. */
    size_t count;
} heap_t;

/**
 * Initializes a heap. The capacity of this heap is initially zero.
 */
heap_t *heap_init();

/**
 * Frees elements of the heap-allocated pointers. This does not properly free
 * if the elements cannot be freed through free. However, you shouldn't need to
 * make anything that does not satisfy this property.
 * @param heap array of heap-allocated pointers
 */
void free_heap(heap_t *heap);