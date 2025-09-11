#include <stdbool.h>
#include <stddef.h>

typedef struct Block Block;

struct Block {
    void *addr;
    Block *next;
    size_t size;
};

typedef struct {
    Block *free;   // first free block
    Block *used;   // first used block
    Block *fresh;  // first available blank block
    void* top;    // top free addr
    Block *blocks;
} Heap;

struct tinyalloc {
    Heap *heap;
    void *heap_limit;
    size_t heap_split_thresh;
    size_t heap_alignment;
    size_t heap_max_blocks;
};

void ta_init(struct tinyalloc *a, const void *base, const void *limit,
             const size_t heap_blocks, const size_t split_thresh,
             const size_t alignment);

void *ta_alloc(struct tinyalloc *a, size_t num);
void *ta_calloc(struct tinyalloc *a, size_t num, size_t size);
void  ta_free(struct tinyalloc *a, void *ptr);
int   ta_posix_memalign(struct tinyalloc *a, void **memptr, size_t align,
			size_t size);

size_t ta_num_free(struct tinyalloc *a);
size_t ta_num_used(struct tinyalloc *a);
size_t ta_num_fresh(struct tinyalloc *a);
bool   ta_check(struct tinyalloc *a);
