#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include "cheri_helpers.h"

#include "meta.h"

LOCK_OBJ_DEF;

#ifdef __CHERI_PURE_CAPABILITY__
const uint16_t size_classes[] = {
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 12, 14,
	18, 20, 25, 30,
	36, 42, 50, 62,
	72, 84, 102, 126,
	146, 170, 204, 254,
	292, 340, 409, 510,
	584, 682, 818, 1022,
	1169, 1364, 1637, 2046,
	2340, 2730, 3276, 4094,
	4680, 5460, 6552, 8190,
};
#else
const uint16_t size_classes[] = {
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 12, 15,
	18, 20, 25, 31,
	36, 42, 50, 63,
	72, 84, 102, 127,
	146, 170, 204, 255,
	292, 340, 409, 511,
	584, 682, 818, 1023,
	1169, 1364, 1637, 2047,
	2340, 2730, 3276, 4095,
	4680, 5460, 6552, 8191,
};
#endif

// There's a hidden dependency on the biggest sizeclass not being (much) above 8k.
// there are 32 slots in one group, so that make the start of the last biggest group
// just below 64k. The offset to access the start of each slot is encoded with 16 bits

static const uint8_t small_cnt_tab[][3] = {
	{ 30, 30, 30 },
	{ 31, 15, 15 },
	{ 20, 10, 10 },
	{ 31, 15, 7 },
	{ 25, 12, 6 },
	{ 21, 10, 5 },
	{ 18, 8, 4 },
	{ 31, 15, 7 },
	{ 28, 14, 6 },
};

static const uint8_t med_cnt_tab[4] = { 28, 24, 20, 32 };

struct malloc_context ctx = { 0 };

struct meta *alloc_meta(void)
{
	struct meta *m;
	unsigned char *p;
	if (!ctx.init_done) {
#ifndef PAGESIZE
		ctx.pagesize = get_page_size();
#endif

#ifdef __CHERI_PURE_CAPABILITY__
		mallocmap_create(1024, &(ctx.capmap));
#endif

		ctx.secret = get_random_secret();
		ctx.init_done = 1;
	}
	size_t pagesize = PGSZ;
	if (pagesize < 4096) pagesize = 4096;
	if ((m = dequeue_head(&ctx.free_meta_head))) return m;
	if (!ctx.avail_meta_count) {
		int need_unprotect = 1;
#ifndef __CHERI_PURE_CAPABILITY__
		if (!ctx.avail_meta_area_count && ctx.brk!=-1) {
			uintptr_t new = ctx.brk + pagesize;
			int need_guard = 0;
			if (!ctx.brk) {
				need_guard = 1;
				ctx.brk = brk(0);
				// some ancient kernels returned _ebss
				// instead of next page as initial brk.
				ctx.brk += -ctx.brk & (pagesize-1);
				new = ctx.brk + 2*pagesize;
			}
			if (brk(new) != new) {
				ctx.brk = -1;
			} else {
				if (need_guard) mmap((void *)ctx.brk, pagesize,
					PROT_NONE, MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
				ctx.brk = new;
				ctx.avail_meta_areas = (void *)(new - pagesize);
				ctx.avail_meta_area_count = pagesize>>12;
				need_unprotect = 0;
			}
		}
#endif
		if (!ctx.avail_meta_area_count) {
			size_t n = 2UL << ctx.meta_alloc_shift;
#ifdef __CHERI_PURE_CAPABILITY__
			p = mmap(0, n*pagesize, PROT_NONE | PROT_MAX(PROT_READ | PROT_WRITE),
				MAP_PRIVATE|MAP_ANON, -1, 0);
#else
			p = mmap(0, n*pagesize, PROT_NONE,
				MAP_PRIVATE|MAP_ANON, -1, 0);
#endif
			if (p==MAP_FAILED) return 0;
#ifdef __CHERI_PURE_CAPABILITY__
			p = restrict_perms(p);
			/* TODO: Remove after mmap implementation. */
			p = __builtin_cheri_bounds_set(p, n*pagesize);
			p = __builtin_cheri_perms_and(p, MUSL_CAP_PROT_MALLOC);
#endif
			ctx.avail_meta_areas = p + pagesize;
			ctx.avail_meta_area_count = (n-1)*(pagesize>>12);
			ctx.meta_alloc_shift++;
		}
		p = ctx.avail_meta_areas;
		if ((uintptr_t)p & (pagesize-1)) need_unprotect = 0;
		if (need_unprotect)
			if (mprotect(p, pagesize, PROT_READ|PROT_WRITE)
			    && errno != ENOSYS)
				return 0;
		ctx.avail_meta_area_count--;
		ctx.avail_meta_areas = p + 4096;
		if (ctx.meta_area_tail) {
			ctx.meta_area_tail->next = (void *)p;
		} else {
			ctx.meta_area_head = (void *)p;
		}
		ctx.meta_area_tail = (void *)p;
		ctx.meta_area_tail->check = ctx.secret;
		ctx.avail_meta_count = ctx.meta_area_tail->nslots
			= (4096-sizeof(struct meta_area))/sizeof *m;
		ctx.avail_meta = ctx.meta_area_tail->slots;
	}
	ctx.avail_meta_count--;
	m = ctx.avail_meta++;
	m->prev = m->next = 0;
	return m;
}

static uint32_t try_avail(struct meta **pm)
{
	struct meta *m = *pm;
	uint32_t first;
	if (!m) return 0;
	uint32_t mask = m->avail_mask;
	if (!mask) {
		if (!m) return 0; //TODO isn't this redundant with 3 line above ?
		if (!m->freed_mask) {
			dequeue(pm, m);
			m = *pm;
			if (!m) return 0;
		} else {
			m = m->next;
			*pm = m;
		}

		mask = m->freed_mask;

		// skip fully-free group unless it's the only one
		// or it's a permanently non-freeable group
		if (mask == (2u<<m->last_idx)-1 && m->freeable) {
			m = m->next;
			*pm = m;
			mask = m->freed_mask;
		}

		// activate more slots in a not-fully-active group
		// if needed, but only as a last resort. prefer using
		// any other group with free slots. this avoids
		// touching & dirtying as-yet-unused pages.
		if (!(mask & ((2u<<m->mem->active_idx)-1))) {
			if (m->next != m) {
				m = m->next;
				*pm = m;
			} else {
				int cnt = m->mem->active_idx + 2;
				int size = size_classes[m->sizeclass]*UNIT;
				int span = GRP_SIZE + size*cnt;
				// activate up to next 4k boundary
				while ((span^(span+size-1)) < 4096) {
					cnt++;
					span += size;
				}
				if (cnt > m->last_idx+1)
					cnt = m->last_idx+1;
				m->mem->active_idx = cnt-1;
			}
		}
		mask = activate_group(m);
		assert(mask);
		decay_bounces(m->sizeclass);
	}
	first = mask&-mask;
	m->avail_mask = mask-first;
	return first;
}

static int alloc_slot(int, size_t);

static struct meta *alloc_group(int sc, size_t req)
{
	size_t size = UNIT*size_classes[sc];
	int i = 0, cnt;
	unsigned char *p;
	struct meta *m = alloc_meta();
	if (!m) return 0;
	size_t usage = ctx.usage_by_class[sc];
	size_t pagesize = PGSZ;
	int active_idx;
	if (sc < 9) {
		while (i<2 && 4*small_cnt_tab[sc][i] > usage)
			i++;
		cnt = small_cnt_tab[sc][i];
	} else {
		// lookup max number of slots fitting in power-of-two size
		// from a table, along with number of factors of two we
		// can divide out without a remainder or reaching 1.
		cnt = med_cnt_tab[sc&3];

		// reduce cnt to avoid excessive eagar allocation.
		while (!(cnt&1) && 4*cnt > usage)
			cnt >>= 1;

		// data structures don't support groups whose slot offsets
		// in units don't fit in 16 bits.
		while (size*cnt >= 65536*UNIT)
			cnt >>= 1;
	}

	// If we selected a count of 1 above but it's not sufficient to use
	// mmap, increase to 2. Then it might be; if not it will nest.
	if (cnt==1 && size*cnt <= (pagesize-GRP_SIZE)/2) cnt = 2;

	// All choices of size*cnt are "just below" a power of two, so anything
	// larger than half the page size should be allocated as whole pages.
	if (size*cnt > (pagesize- GRP_SIZE)/2) {
		// check/update bounce counter to start/increase retention
		// of freed maps, and inhibit use of low-count, odd-size
		// small mappings and single-slot groups if activated.
		int nosmall = is_bouncing(sc);
		account_bounce(sc);
		step_seq();

		// since the following count reduction opportunities have
		// an absolute memory usage cost, don't overdo them. count
		// coarse usage as part of usage.
		if (!(sc&1) && sc<32) usage += ctx.usage_by_class[sc+1];

		// try to drop to a lower count if the one found above
		// increases usage by more than 25%. these reduced counts
		// roughly fill an integral number of pages, just not a
		// power of two, limiting amount of unusable space.
		if (4*cnt > usage && !nosmall) {
			if (0);
			else if ((sc&3)==1 && size*cnt>8*pagesize) cnt = 2;
			else if ((sc&3)==2 && size*cnt>4*pagesize) cnt = 3;
			else if ((sc&3)==0 && size*cnt>8*pagesize) cnt = 3;
			else if ((sc&3)==0 && size*cnt>2*pagesize) cnt = 5;
		}
		size_t needed = size*cnt + GRP_SIZE;
		needed += -needed & (pagesize-1);

		// produce an individually-mmapped allocation if usage is low,
		// bounce counter hasn't triggered, and either it saves memory
		// or it avoids eagar slot allocation without wasting too much.
		if (!nosmall && cnt<=7) {
			req += IB + GRP_SIZE;
			req += -req & (pagesize-1);
			if (req<size+GRP_SIZE || (req>=4*pagesize && 2*cnt>usage)) {
				cnt = 1;
				needed = req;
			}
		}

		p = mmap(0, needed, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
		if (p==MAP_FAILED) {
			free_meta(m);
			return 0;
		}
#ifdef __CHERI_PURE_CAPABILITY__
		p = restrict_perms(p);
		/* TODO: Remove after mmap implementation. */
		p = __builtin_cheri_bounds_set(p, needed);
		p = __builtin_cheri_perms_and(p, MUSL_CAP_PROT_MALLOC);
#endif
		m->maplen = needed>>12;
		ctx.mmap_counter++;
		active_idx = (4096-GRP_SIZE)/size-1;
		if (active_idx > cnt-1) active_idx = cnt-1;
		if (active_idx < 0) active_idx = 0;
	} else {
		int j = size_to_class(GRP_SIZE+cnt*size-IB);
		int idx = alloc_slot(j, GRP_SIZE+cnt*size-IB);
		if (idx < 0) {
			free_meta(m);
			return 0;
		}
		struct meta *g = ctx.active[j];
		p = enframe(g, idx, UNIT*size_classes[j]-IB, ctx.mmap_counter, UNIT);
		m->maplen = 0;
		p[-3] = (p[-3]&31) | (6<<5);
		for (int i=0; i<=cnt; i++)
			p[GRP_SIZE+i*size-4] = 0;
		active_idx = cnt-1;
	}
	ctx.usage_by_class[sc] += cnt;
	m->avail_mask = (2u<<active_idx)-1;
	m->freed_mask = (2u<<(cnt-1))-1 - m->avail_mask;
	m->mem = (void *)p;
	m->mem->meta = m;
	m->mem->active_idx = active_idx;
	m->last_idx = cnt-1;
	m->freeable = 1;
	m->sizeclass = sc;
	return m;
}

static int alloc_slot(int sc, size_t req)
{
	uint32_t first = try_avail(&ctx.active[sc]);
	if (first) return a_ctz_32(first);

	struct meta *g = alloc_group(sc, req);
	if (!g) return -1;

	g->avail_mask--;
	queue(&ctx.active[sc], g);
	return 0;
}

void *malloc_aligned(size_t n, size_t align)
{
	if (size_overflows(n)) return 0;
	struct meta *g;
	uint32_t mask, first;
	int sc;
	int idx;
	int ctr;

	size_t padded_n = n;
	if (align > UNIT) {
		padded_n += align;
	}

	if (padded_n >= MMAP_THRESHOLD) {
		// mmap must contain enough room for group structure information
		size_t needed = padded_n + IB + GRP_SIZE;
		void *p = mmap(0, needed, PROT_READ|PROT_WRITE,
			MAP_PRIVATE|MAP_ANON, -1, 0);
		if (p==MAP_FAILED) return 0;
#ifdef __CHERI_PURE_CAPABILITY__
		p = restrict_perms(p);
		/* TODO: Remove after mmap implementation. */
		p = __builtin_cheri_bounds_set(p, ((needed+4095)/4096) * 4096);
		p = __builtin_cheri_perms_and(p, MUSL_CAP_PROT_MALLOC);
#endif
		wrlock();
		step_seq();
		g = alloc_meta();
		if (!g) {
			unlock();
			munmap(p, needed);
			return 0;
		}
		g->mem = p;
		g->mem->meta = g;
		g->last_idx = 0;
		g->freeable = 1;
		g->sizeclass = 63;
		g->maplen = (needed+4095)/4096;
		g->avail_mask = g->freed_mask = 0;
		// use a global counter to cycle offset in
		// individually-mmapped allocations.
		ctx.mmap_counter++;
		idx = 0;
		goto success;
	}

	sc = size_to_class(padded_n);

	rdlock();
	g = ctx.active[sc];

	// use coarse size classes initially when there are not yet
	// any groups of desired size. this allows counts of 2 or 3
	// to be allocated at first rather than having to start with
	// 7 or 5, the min counts for even size classes.
	if (!g && sc>=4 && sc<32 && sc!=6 && !(sc&1) && !ctx.usage_by_class[sc]) {
		size_t usage = ctx.usage_by_class[sc|1];
		// if a new group may be allocated, count it toward
		// usage in deciding if we can use coarse class.
		if (!ctx.active[sc|1] || (!ctx.active[sc|1]->avail_mask
		    && !ctx.active[sc|1]->freed_mask))
			usage += 3;
		if (usage <= 12)
			sc |= 1;
		g = ctx.active[sc];
	}

	for (;;) {
		mask = g ? g->avail_mask : 0;
		first = mask&-mask;
		if (!first) break;
		if (RDLOCK_IS_EXCLUSIVE || !MT)
			g->avail_mask = mask-first;
		else if (a_cas(&g->avail_mask, mask, mask-first)!=mask)
			continue;
		idx = a_ctz_32(first);
		goto success;
	}
	upgradelock();

	idx = alloc_slot(sc, padded_n);
	if (idx < 0) {
		unlock();
		return 0;
	}
	g = ctx.active[sc];

success:
	ctr = ctx.mmap_counter;

#ifdef __CHERI_PURE_CAPABILITY__
	void *p = enframe(g, idx, n, ctr, align);
	p = restrict_user_ptr(p, n);

	int r = mallocmap_insert(p, g->mem, &(ctx.capmap));
	if (!r) {
		return 0;
	}
#endif

	unlock();

#ifdef __CHERI_PURE_CAPABILITY__
	return p;
#else
	return enframe(g, idx, n, ctr, align);
#endif
}

void *malloc(size_t n) {
#ifdef __CHERI_PURE_CAPABILITY__
	size_t cap_n = __builtin_cheri_round_representable_length(n);
	// Never decrease the size
	if (cap_n > n) n = cap_n;
	size_t align = ~__builtin_cheri_representable_alignment_mask(n) + 1;
	if (align < UNIT) align = UNIT;
#else
	size_t align = UNIT;
#endif

	return malloc_aligned(n, align);
}

int is_allzero(void *p)
{
#ifdef __CHERI_PURE_CAPABILITY__
	rdlock();
	p = expand_bounds(p);
	unlock();
#endif

	struct meta *g = get_meta(p);
	return g->sizeclass >= 48 ||
		get_stride(g) < UNIT*size_classes[g->sizeclass];
}

#ifdef __CHERI_PURE_CAPABILITY__
// we are not including lite_malloc object anymore
void *__libc_malloc(size_t n)
{
	return __libc_malloc_impl(n);
}
static void *default_malloc(size_t n)
{
	return __libc_malloc_impl(n);
}
#undef malloc
weak_alias(default_malloc, malloc);
#endif
