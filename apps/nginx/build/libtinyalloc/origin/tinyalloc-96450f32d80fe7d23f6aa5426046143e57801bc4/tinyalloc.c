#include "tinyalloc.h"
#include <stdint.h>
#include <uk/assert.h>
#include <sfork.h>
#include <flexos/isolation.h>

#ifdef TA_DEBUG
extern void print_s(char *);
extern void print_i(size_t);
#else
#define print_s(X)
#define print_i(X)
#endif

/**
 * If compaction is enabled, inserts block
 * into free list, sorted by addr.
 * If disabled, add block has new head of
 * the free list.
 */
static void insert_block(struct tinyalloc *a, Block *block) {
    Heap *heap = a->heap;

#ifndef CONFIG_LIBTINYALLOC_DISABLE_COMPACT
    Block *ptr  = heap->free;
    Block *prev = NULL;
    while (ptr != NULL) {
        if ((size_t)block->addr <= (size_t)ptr->addr) {
            print_s("insert");
            print_i((size_t)ptr);
            break;
        }
        prev = ptr;
        ptr  = ptr->next;
    }
    if (prev != NULL) {
        if (ptr == NULL) {
            print_s("new tail");
        }
        prev->next = block;
    } else {
        print_s("new head");
        heap->free = block;
    }
    block->next = ptr;
#else
    if (heap->free == block)
        return;
    block->next = heap->free;
    if (heap->free)
    heap->free->prev = block;
    block->prev = NULL;
    heap->free  = block;
#endif
}

#ifndef CONFIG_LIBTINYALLOC_DISABLE_COMPACT
static void release_blocks(struct tinyalloc *a, Block *scan, Block *to) {
    Heap *heap = a->heap;

    Block *scan_next;
    while (scan != to) {
        print_s("release");
        print_i((size_t)scan);
        scan_next   = scan->next;
        scan->next  = heap->fresh;
        heap->fresh = scan;
        scan->addr  = 0;
        scan->size  = 0;
        scan        = scan_next;
    }
}

static void compact(struct tinyalloc *a) {
    Heap *heap = a->heap;

    Block *ptr = heap->free;
    Block *prev;
    Block *scan;
    while (ptr != NULL) {
        prev = ptr;
        scan = ptr->next;
        while (scan != NULL &&
               (size_t)prev->addr + prev->size == (size_t)scan->addr) {
            print_s("merge");
            print_i((size_t)scan);
            prev = scan;
            scan = scan->next;
        }
        if (prev != ptr) {
            size_t new_size =
                (size_t)prev->addr - (size_t)ptr->addr + prev->size;
            print_s("new size");
            print_i(new_size);
            ptr->size   = new_size;
            Block *next = prev->next;
            // make merged blocks available
            release_blocks(a, ptr->next, prev->next);
            // relink
            ptr->next = next;
        }
        ptr = ptr->next;
    }
}
#endif

void ta_init(struct tinyalloc *a, const void *base, const void *limit,
             const size_t heap_blocks, const size_t split_thresh,
             const size_t alignment) {
    Heap *heap;

    a->heap = (Heap *)base;
    a->heap_limit = limit;
    a->heap_split_thresh = split_thresh;
    a->heap_alignment = alignment;
    a->heap_max_blocks = heap_blocks;

    heap = a->heap;
    heap->free   = NULL;
    heap->used   = NULL;
    heap->blocks = (Block*) (base + sizeof(Heap));
    heap->fresh  = heap->blocks;
    heap->top    = base + sizeof(Heap) + heap_blocks * sizeof(Block);

    Block *block = heap->blocks;
    size_t i     = a->heap_max_blocks - 1;
    Block *prev = NULL;
    while (i--) {
        block->next = block + 1;
        block->prev = prev;
        prev = block;
        block++;
    }
    block->next = NULL;
}

void ta_free(struct tinyalloc *a, void *free) {
    Heap *heap = a->heap;

    Block *block = heap->used;
    Block *prev  = NULL;
    uintptr_t* addr = int_to_cap(__builtin_cheri_base_get(free));
    //addr--;
    block = (Block*)*addr;
    if (block->addr == __builtin_cheri_base_get(free)) {
        if (block->prev) {
            block->prev->next = block->next;
            if (block->next)
                block->next->prev = block->prev;
        } else {
            heap->used = block->next;
            if (block->next)
                block->next->prev = NULL;
        }
        
       // uk_pr_crit("Did we do this? block->prev %p, block->next %p\n", block->prev, block->next);
        insert_block(a, block);
        return;
    }
    block = heap->used;
    while (block != NULL) {
        if (__builtin_cheri_base_get(free) == block->addr) {
            // if (block->size > 100000)
             //uk_pr_crit("it has worked, size %d\n", block->size);
            if (prev) {
                prev->next = block->next;
                block->next->prev = prev;
            } else {
                heap->used = block->next;
                block->next->prev = NULL;
            }
            insert_block(a, block);
#ifndef CONFIG_LIBTINYALLOC_DISABLE_COMPACT
            compact(a);
#endif
            return;
       }
        prev  = block;
        block = block->next;
    }
   // uk_pr_crit("free don't work init %p\n", free);
    return;
}

static Block *alloc_block(struct tinyalloc *a, size_t num) {
    Heap *heap = a->heap;

    //uk_pr_crit("Heap %p, num %d, pid %d\n", heap, num, PROCESS_ID());

    Block *ptr  = heap->free;
    Block *prev = NULL;
    void* top  = heap->top;
    num         = (num + a->heap_alignment - 1) & - a->heap_alignment;
    while (ptr != NULL) {
      //  uk_pr_crit("ptr %p\n", ptr);
        const int is_top = ((size_t)ptr->addr + ptr->size >= top)
		        && ((size_t)ptr->addr + num <= a->heap_limit);
        if (is_top || ptr->size >= num) {
            if (prev != NULL) {
                prev->next = ptr->next;
            if (ptr->next)
                ptr->next->prev = prev;
            } else {
                heap->free = ptr->next;
                if (ptr->next)
                    ptr->next->prev = NULL;
            }
            if (heap->used)
                heap->used->prev = ptr;
            ptr->next  = heap->used;
            ptr->prev = NULL;
            heap->used = ptr;
            if (is_top) {
                print_s("resize top block");
                ptr->size = num;
                heap->top = ptr->addr + num;
// #ifndef CONFIG_LIBTINYALLOC_DISABLE_SPLIT
//             } else if (heap->fresh != NULL) {
//                 size_t excess = ptr->size - num;
//                 if (excess >= a->heap_split_thresh) {
//                     ptr->size    = num;
//                     Block *split = heap->fresh;
//                     heap->fresh  = split->next;
//                     split->addr  = (void *)(ptr->addr + num);
//                     print_s("split");
//                     print_i(split->addr);
//                     split->size = excess;
//                     insert_block(a, split);
// #ifndef CONFIG_LIBTINYALLOC_DISABLE_COMPACT
//                     compact(a);
// #endif
//                 }
// #endif
            }
            // if (ptr->size > 100000)
            //     uk_pr_crit("ptr size %d\n", ptr->size);
            return ptr;
        }
        prev = ptr;
        ptr  = ptr->next;
        if (prev == ptr) {

        }
    }

    // no matching free blocks
    // see if any other blocks available
    void* new_top = top + num;
    if (heap->fresh != NULL && new_top <= (size_t)a->heap_limit) {
        ptr         = heap->fresh;
        heap->fresh = ptr->next;
        if (heap->fresh)
        heap->fresh->prev = NULL;
        ptr->addr   = (void *)top;
        if (heap->used)
        heap->used->prev = ptr;
        ptr->prev = NULL;
        ptr->next   = heap->used;
        ptr->size   = num;
        heap->used  = ptr;
        heap->top   = new_top;
        // if (ptr->size > 100000)
        //         uk_pr_crit("ptr size %d\n", ptr->size);
        return ptr;
    }
    uk_pr_crit("Nothing is free, heap %p, pid %d\n", heap, PROCESS_ID());
    return NULL;
}

void *ta_alloc(struct tinyalloc *a, size_t num) {
    Block *block = alloc_block(a, num);
    if (block != NULL) {
        // if (num > 4000)
        // uk_pr_crit("num %d\n", num);
        UK_ASSERT((((uintptr_t)block->addr)%16) == 0);
        //void *ptr = __builtin_cheri_offset_set(block->addr, (uintptr_t)block->addr);
        uintptr_t* *ptr = block->addr;
        *ptr = (void*)block;
        
	    ptr = __builtin_cheri_bounds_set(ptr, (num + a->heap_alignment - 1) & - a->heap_alignment);
        ptr++;
        UK_ASSERT(__builtin_cheri_tag_get(ptr) == 1);
        //uk_pr_crit("alloc: ptr %p, ptr base %p, ptr bounds %p\n", ptr, __builtin_cheri_base_get(ptr), ptr + (__builtin_cheri_length_get(ptr) - __builtin_cheri_offset_get(ptr)));
        return ptr;
    }
    uk_pr_crit("ah interesting, num %d\n", num);
    return NULL;
}

static void memclear(void *ptr, size_t num) {
    size_t *ptrw = (size_t *)ptr;
    size_t numw  = (num & -sizeof(size_t)) / sizeof(size_t);
    while (numw--) {
        *ptrw++ = 0;
    }
    num &= (sizeof(size_t) - 1);
    uint8_t *ptrb = (uint8_t *)ptrw;
    while (num--) {
        *ptrb++ = 0;
    }
}

void *ta_calloc(struct tinyalloc *a, size_t num, size_t size) {
    num *= size;
    Block *block = alloc_block(a, num);
    if (block != NULL) {
        memclear(block->addr, num);
        UK_ASSERT((((uintptr_t)block->addr)%16) == 0);
        void *ptr = block->addr;
	    ptr = __builtin_cheri_bounds_set(ptr, (num + a->heap_alignment - 1) & - a->heap_alignment);
        return ptr;
    }
    return NULL;
}

static size_t count_blocks(Block *ptr) {
    size_t num = 0;
    while (ptr != NULL) {
        num++;
        ptr = ptr->next;
    }
    return num;
}

size_t ta_num_free(struct tinyalloc *a) {
    Heap *heap = a->heap;
    return count_blocks(heap->free);
}

size_t ta_num_used(struct tinyalloc *a) {
    Heap *heap = a->heap;
    return count_blocks(heap->used);
}

size_t ta_num_fresh(struct tinyalloc *a) {
    Heap *heap = a->heap;
    return count_blocks(heap->fresh);
}

bool ta_check(struct tinyalloc *a) {
    return a->heap_max_blocks == ta_num_free(a) + ta_num_used(a) + ta_num_fresh(a);
}
