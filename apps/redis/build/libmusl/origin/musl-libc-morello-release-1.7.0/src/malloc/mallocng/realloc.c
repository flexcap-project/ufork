#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "meta.h"

void *realloc(void *p, size_t n)
{
	if (!p) return malloc(n);

	// preserve original user pointer and requested size
	void *userp = p;
	size_t req_n = n;

#ifdef __CHERI_PURE_CAPABILITY__
	rdlock();
	p = expand_bounds(p);
	unlock();
#endif

	if (size_overflows(n)) return 0;

	struct meta *g = get_meta(p);
	int idx = get_slot_index(p);
	size_t stride = get_stride(g);
	unsigned char *start = g->mem->storage + stride*idx;
	unsigned char *end = start + stride - IB;
	size_t old_size = get_nominal_size(p, end);
	size_t avail_size = end-(unsigned char *)p;
	void *new;

#ifdef __CHERI_PURE_CAPABILITY__
	n = __builtin_cheri_round_representable_length(n);
	size_t new_alignment = __builtin_cheri_representable_alignment_mask(n);

	if ((new_alignment & (ptraddr_t) userp) != (ptraddr_t) userp) {
		// alignment has to change, just fall back to malloc and free
		goto malloc_then_free;
	}
#endif

	// only resize in-place if size class matches
	if (n <= avail_size && n<MMAP_THRESHOLD
	    && size_to_class(n)+1 >= g->sizeclass) {
		set_size(p, end, n);

#ifdef __CHERI_PURE_CAPABILITY__
		wrlock();
		mallocmap_delete(userp, &(ctx.capmap));
		p = restrict_user_ptr(p, n);
		mallocmap_insert(p, g->mem, &(ctx.capmap));
		unlock();
#endif
		return p;
	}

	// use mremap if old and new size are both mmap-worthy
	if (g->sizeclass>=48 && n>=MMAP_THRESHOLD) {
		assert(g->sizeclass==63);
		size_t base = (unsigned char *)p-start;
		size_t needed = (n + base + GRP_SIZE + IB + 4095) & -4096;
		new = g->maplen*4096UL == needed ? g->mem :
			mremap(g->mem, g->maplen*4096UL, needed, MREMAP_MAYMOVE);
		if (new!=MAP_FAILED) {
#ifdef __CHERI_PURE_CAPABILITY__
			new = restrict_perms(new);
#endif
			g->mem = new;
			g->maplen = needed/4096;
			p = g->mem->storage + base;
			end = g->mem->storage + (needed - GRP_SIZE) - IB;
			*end = 0;
			set_size(p, end, n);
#ifdef __CHERI_PURE_CAPABILITY__
			wrlock();
			mallocmap_delete(userp, &(ctx.capmap));
			p = restrict_user_ptr(p, n);
			mallocmap_insert(p, g->mem, &(ctx.capmap));
			unlock();
#endif
			return p;
		}
	}

malloc_then_free:
	new = malloc(req_n);
	if (!new) return 0;
	memcpy(new, p, req_n < old_size ? req_n : old_size);
	free(userp);

	return new;
}
