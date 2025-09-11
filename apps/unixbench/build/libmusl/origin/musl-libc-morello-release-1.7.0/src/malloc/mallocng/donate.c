#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "meta.h"

static void donate(unsigned char *base, size_t len)
{
	uintptr_t a = (uintptr_t)base;
	uintptr_t b = a + len;
	a += -a & (UNIT-1);
	b -= b & (UNIT-1);
	memset(base, 0, len);
	for (int sc=47; sc>0 && b>a; sc-=4) {
		if (b-a < (size_classes[sc]*UNIT+GRP_SIZE)) continue;
		struct meta *m = alloc_meta();
		m->avail_mask = 0;
		m->freed_mask = 1;
#ifdef __CHERI_PURE_CAPABILITY__
		void *restricted_ptr = restrict_user_ptr(a, size_classes[sc]*UNIT+GRP_SIZE);
		restricted_ptr = restrict_perms(restricted_ptr);
		m->mem = restricted_ptr;
#else
		m->mem = (void *)a;
#endif
		m->mem->meta = m;
		m->last_idx = 0;
		m->freeable = 0;
		m->sizeclass = sc;
		m->maplen = 0;
		*((unsigned char *)m->mem+GRP_SIZE-4) = 0;
		*((unsigned char *)m->mem+GRP_SIZE-3) = 255;
		m->mem->storage[size_classes[sc]*UNIT-4] = 0;
		queue(&ctx.active[sc], m);
		a += size_classes[sc]*UNIT+GRP_SIZE;
	}
}

void __malloc_donate(char *start, char *end)
{
	donate((void *)start, end-start);
}
