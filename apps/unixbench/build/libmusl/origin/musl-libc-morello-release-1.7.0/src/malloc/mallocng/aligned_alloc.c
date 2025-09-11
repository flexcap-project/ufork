#include <stdlib.h>
#include <errno.h>
#include "meta.h"

void *aligned_alloc(size_t align, size_t len)
{
	if ((align & -align) != align) {
		errno = EINVAL;
		return 0;
	}

	if (len > SIZE_MAX - align || align >= (1ULL<<31)*UNIT) {
		errno = ENOMEM;
		return 0;
	}

	if (DISABLE_ALIGNED_ALLOC) {
		errno = ENOMEM;
		return 0;
	}

	if (align <= UNIT) align = UNIT;

#ifdef __CHERI_PURE_CAPABILITY__
	size_t morello_len = __builtin_cheri_round_representable_length(len);
	size_t morello_alignment = ~__builtin_cheri_representable_alignment_mask(morello_len) + 1;

	// If morello alignment is larger, malloc is already going to align this to a more strict alignment
	//  anyway. We can essentially do nothing.
	if (morello_alignment >= align) {
		return malloc(len);
	}

	len = morello_len;
#endif

	return malloc_aligned(len, align);
}
