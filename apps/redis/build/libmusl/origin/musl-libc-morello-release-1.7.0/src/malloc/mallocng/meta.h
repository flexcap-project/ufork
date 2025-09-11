#ifndef MALLOC_META_H
#define MALLOC_META_H

#define _GNU_SOURCE

#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <search.h>

#include "glue.h"
#include "mallocmap.h"

__attribute__((__visibility__("hidden")))
extern const uint16_t size_classes[];

// MMAP THRESHOLD is created by (UNIT * max size class) - IB
#ifdef __CHERI_PURE_CAPABILITY__
#define MMAP_THRESHOLD 131036
#else
#define MMAP_THRESHOLD 131052
#endif

#ifdef __CHERI_PURE_CAPABILITY__
#define GRP_SIZE 32
#else
#define GRP_SIZE 16
#endif

#define UNIT 16

#define IB 4

struct group {
	struct meta *meta;
	unsigned char active_idx:5;
	char pad[GRP_SIZE - sizeof(struct meta *) - 1];
	unsigned char storage[];
};

struct meta {
	struct meta *prev, *next;
	struct group *mem;
	volatile int avail_mask, freed_mask;
	size_t last_idx:5;
	size_t freeable:1;
	size_t sizeclass:6;
	size_t maplen:8*sizeof(size_t)-12;
};

struct meta_area {
	uint64_t check;
	struct meta_area *next;
	int nslots;
	struct meta slots[];
};

struct malloc_context {
	uint64_t secret;
#ifndef PAGESIZE
	size_t pagesize;
#endif
	int init_done;
	unsigned mmap_counter;
	struct meta *free_meta_head;
	struct meta *avail_meta;
	size_t avail_meta_count, avail_meta_area_count, meta_alloc_shift;
	struct meta_area *meta_area_head, *meta_area_tail;
	unsigned char *avail_meta_areas;
	struct meta *active[48];
	size_t usage_by_class[48];
	uint8_t unmap_seq[32], bounces[32];
	uint8_t seq;
#ifdef __CHERI_PURE_CAPABILITY__
	uintptr_t __padding;
	struct __mallocmap_tab capmap;
#else
	uintptr_t brk;
#endif
};

__attribute__((__visibility__("hidden")))
extern struct malloc_context ctx;

#ifdef PAGESIZE
#define PGSZ PAGESIZE
#else
#define PGSZ ctx.pagesize
#endif

__attribute__((__visibility__("hidden")))
struct meta *alloc_meta(void);

__attribute__((__visibility__("hidden")))
int is_allzero(void *);

#ifdef __CHERI_PURE_CAPABILITY__
static const unsigned long USER_PTR_PERMS_REMOVED =
#ifdef __ARM_CAP_PERMISSION_EXECUTIVE__
	__ARM_CAP_PERMISSION_EXECUTIVE__ |
#endif
#ifdef __ARM_CAP_PERMISSION_COMPARTMENT_ID__
	__ARM_CAP_PERMISSION_COMPARTMENT_ID__ |
#endif
#ifdef __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__
	__ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__ |
#endif
	__CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__ |
	__CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ |
	__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
	__CHERI_CAP_PERMISSION_PERMIT_EXECUTE__;

static inline void *expand_bounds(void *p) {
	struct group *g = (struct group *) mallocmap_find(p, &(ctx.capmap));
	assert(g);

	return __builtin_cheri_address_set(g, (size_t) p);
}

static inline void *restrict_user_ptr(void *p, size_t len) {
	return __builtin_cheri_perms_and(__builtin_cheri_bounds_set(p, len),
		__CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS | WRITE_CAP_PERMS);
}

static inline void *restrict_perms(void *p) {
	return __builtin_cheri_perms_and(p, ~USER_PTR_PERMS_REMOVED);
}
#endif

static inline void queue(struct meta **phead, struct meta *m)
{
	assert(!m->next);
	assert(!m->prev);
	if (*phead) {
		struct meta *head = *phead;
		m->next = head;
		m->prev = head->prev;
		m->next->prev = m->prev->next = m;
	} else {
		m->prev = m->next = m;
		*phead = m;
	}
}

static inline void dequeue(struct meta **phead, struct meta *m)
{
	if (m->next != m) {
		m->prev->next = m->next;
		m->next->prev = m->prev;
		if (*phead == m) *phead = m->next;
	} else {
		*phead = 0;
	}
	m->prev = m->next = 0;
}

static inline struct meta *dequeue_head(struct meta **phead)
{
	struct meta *m = *phead;
	if (m) dequeue(phead, m);
	return m;
}

static inline void free_meta(struct meta *m)
{
	*m = (struct meta){0};
	queue(&ctx.free_meta_head, m);
}

static inline uint32_t activate_group(struct meta *m)
{
	assert(!m->avail_mask);
	uint32_t mask, act = (2u<<m->mem->active_idx)-1;
	do mask = m->freed_mask;
	while (a_cas(&m->freed_mask, mask, mask&~act)!=mask);
	return m->avail_mask = mask & act;
}

static inline int get_slot_index(const unsigned char *p)
{
	return p[-3] & 31;
}

static inline struct meta *get_meta(const unsigned char *p)
{
	assert(!((uintptr_t)p & 15));
	int offset = *(const uint16_t *)(p - 2);
	int index = get_slot_index(p);
	if (p[-4]) {
		assert(!offset);
		offset = *(uint32_t *)(p - 8);
		assert(offset > 0xffff);
	}
	const struct group *base = (const void *)(p - UNIT*offset - GRP_SIZE);
	const struct meta *meta = base->meta;
	assert(meta->mem == base);
	assert(index <= meta->last_idx);
	assert(!(meta->avail_mask & (1u<<index)));
	assert(!(meta->freed_mask & (1u<<index)));
	const struct meta_area *area = (void *)((uintptr_t)meta & -4096);
	assert(area->check == ctx.secret);
	if (meta->sizeclass < 48) {
		assert(offset >= size_classes[meta->sizeclass]*index);
		assert(offset < size_classes[meta->sizeclass]*(index+1));
	} else {
		assert(meta->sizeclass == 63);
	}
	if (meta->maplen) {
		assert(offset <= meta->maplen*4096UL/UNIT - 1);
	}
	return (struct meta *)meta;
}

static inline size_t get_nominal_size(const unsigned char *p, const unsigned char *end)
{
	size_t reserved = p[-3] >> 5;
	if (reserved >= 5) {
		assert(reserved == 5);
		reserved = *(const uint32_t *)(end-4);
		assert(reserved >= 5);
		assert(!end[-5]);
	}
	assert(reserved <= end-p);
	assert(!*(end-reserved));
	// also check the slot's overflow byte
	assert(!*end);
	return end-reserved-p;
}

static inline size_t get_stride(const struct meta *g)
{
	if (!g->last_idx && g->maplen) {
		return g->maplen*4096UL - GRP_SIZE;
	} else {
		return UNIT*size_classes[g->sizeclass];
	}
}

static inline void set_size(unsigned char *p, unsigned char *end, size_t n)
{
	int reserved = end-p-n; // reserved is what "slack" we are left after accounting the offset ?
	if (reserved) end[-reserved] = 0; //and so we force writing a 0 after the last byte the user has requested
	if (reserved >= 5) { //if reserved is too big (we only have 3 bits to represent it at p-3)
		*(uint32_t *)(end-4) = reserved; // then we store it near the end
		end[-5] = 0; // write the null byte
		reserved = 5; // and set reserved to the special value to indicate one must read the field at end-4
	}
	p[-3] = (p[-3]&31) + (reserved<<5);
}

static inline void *enframe(struct meta *g, int idx, size_t n, int ctr, size_t align)
{
	size_t stride = get_stride(g);
	size_t slack = (stride-IB-n)/UNIT;
	unsigned char *p = g->mem->storage + stride*idx;
	unsigned char *end = p+stride-IB;
	// cycle offset within slot to increase interval to address
	// reuse, facilitate trapping double-free.
	int off = (p[-3] ? *(uint16_t *)(p-2) + 1 : ctr) & 255;

	if (align > UNIT) {
		unsigned char *aligned_p = __builtin_align_up(p, align);
		off = (aligned_p - p) / UNIT;
	}

	assert(!p[-4]);
	if (off > slack) {
		size_t m = slack;
		m |= m>>1; m |= m>>2; m |= m>>4;
		off &= m;
		if (off > slack) off -= slack+1;
		assert(off <= slack);
	}
	assert(g->sizeclass == 63 || off <= size_classes[g->sizeclass] * (idx+1));
	if (off) {
		// store offset in unused header at offset zero
		// if enframing at non-zero offset.
		*(uint16_t *)(p-2) = off;
		p[-3] = 7<<5;
		p += UNIT*off;
		// for nonzero offset there is no permanent check
		// byte, so make one.
		p[-4] = 0;
	}
	*(uint16_t *)(p-2) = (size_t)(p-g->mem->storage)/UNIT;
	p[-3] = idx;
	set_size(p, end, n);
	return p;
}

static inline int size_to_class(size_t n)
{
	// shift by 4 to divide by 16 (UNIT)
	n = (n+IB-1)>>4;
	if (n<10) return n;
	n += GRP_SIZE >> 4;
	/*
	 * 28 is the number of leading 0's of the geometric progression.
	 * Largest power of 2 that is smaller than n by counting leading 0's (clz_32).
	 * 28 - clz_32(n) = log2(n) - log2(first of element of geometric progression),
	 * which is the number of rows progressed in the array.
	 * Multiply by 4 as there is 4 elements per size_class row. Each 4th element
	 * is doubled and increased to power of 2 - GRP_SIZE >> 4.
	 * Add 8 as geometric progression is offset by 8 due to 8 linear size_classes.
	 */
	int i = (28-a_clz_32(n))*4 + 8;
	/*
	 * Calculate offset in row.
	 */
	if (n>size_classes[i+1]) i+=2;
	if (n>size_classes[i]) i++;
	return i;
}

static inline int size_overflows(size_t n)
{
	if (n >= SIZE_MAX/2 - 4096) {
		errno = ENOMEM;
		return 1;
	}
	return 0;
}

static inline void step_seq(void)
{
	if (ctx.seq==255) {
		for (int i=0; i<32; i++) ctx.unmap_seq[i] = 0;
		ctx.seq = 1;
	} else {
		ctx.seq++;
	}
}

static inline void record_seq(int sc)
{
	if (sc-7U < 32) ctx.unmap_seq[sc-7] = ctx.seq;
}

static inline void account_bounce(int sc)
{
	if (sc-7U < 32) {
		int seq = ctx.unmap_seq[sc-7];
		if (seq && ctx.seq-seq < 10) {
			if (ctx.bounces[sc-7]+1 < 100)
				ctx.bounces[sc-7]++;
			else
				ctx.bounces[sc-7] = 150;
		}
	}
}

static inline void decay_bounces(int sc)
{
	if (sc-7U < 32 && ctx.bounces[sc-7])
		ctx.bounces[sc-7]--;
}

static inline int is_bouncing(int sc)
{
	return (sc-7U < 32 && ctx.bounces[sc-7] >= 100);
}

#endif
