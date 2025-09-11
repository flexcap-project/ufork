// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 */
#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"
#include <uk/assert.h>

/*
 * Minimal sanity check for a read-only tree. fdt_ro_probe_() checks
 * that the given buffer contains what appears to be a flattened
 * device tree with sane information in its header.
 */
int32_t fdt_ro_probe_(const void *fdt)
{
	UK_ASSERT(__builtin_cheri_tag_get(fdt) == 1);
	uint32_t totalsize = fdt_totalsize(fdt);
	//uk_pr_crit("after totalsize\n");

	if (can_assume(VALID_DTB))
		return totalsize;

	/* The device tree must be at an 8-byte aligned address */
	if ((uintptr_t)fdt & 7)
		return -FDT_ERR_ALIGNMENT;

	//uk_pr_crit("correctly aligned\n");

	if (fdt_magic(fdt) == FDT_MAGIC) {
		/* Complete tree */
		if (!can_assume(LATEST)) {
			if (fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
				return -FDT_ERR_BADVERSION;
			if (fdt_last_comp_version(fdt) >
					FDT_LAST_SUPPORTED_VERSION)
				return -FDT_ERR_BADVERSION;
		}
	//	uk_pr_crit("magical\n");
	} else if (fdt_magic(fdt) == FDT_SW_MAGIC) {
	//	uk_pr_crit("not magical\n");
		/* Unfinished sequential-write blob */
		if (!can_assume(VALID_INPUT) && fdt_size_dt_struct(fdt) == 0)
			return -FDT_ERR_BADSTATE;
	} else {
		return -FDT_ERR_BADMAGIC;
	}
	//uk_pr_crit("at end\n");
	if (totalsize < INT32_MAX)
		return totalsize;
	else
		return -FDT_ERR_TRUNCATED;
}

static int check_off_(uint32_t hdrsize, uint32_t totalsize, uint32_t off)
{
	return (off >= hdrsize) && (off <= totalsize);
}

static int check_block_(uint32_t hdrsize, uint32_t totalsize,
			uint32_t base, uint32_t size)
{
	if (!check_off_(hdrsize, totalsize, base))
		return 0; /* block start out of bounds */
	if ((base + size) < base)
		return 0; /* overflow */
	if (!check_off_(hdrsize, totalsize, base + size))
		return 0; /* block end out of bounds */
	return 1;
}

size_t fdt_header_size_(uint32_t version)
{
	if (version <= 1)
		return FDT_V1_SIZE;
	else if (version <= 2)
		return FDT_V2_SIZE;
	else if (version <= 3)
		return FDT_V3_SIZE;
	else if (version <= 16)
		return FDT_V16_SIZE;
	else
		return FDT_V17_SIZE;
}

size_t fdt_header_size(const void *fdt)
{
	return can_assume(LATEST) ? FDT_V17_SIZE :
		fdt_header_size_(fdt_version(fdt));
}

int fdt_check_header(const void *fdt)
{
	size_t hdrsize;

	/* The device tree must be at an 8-byte aligned address */
	if ((uintptr_t)fdt & 7)
		return -FDT_ERR_ALIGNMENT;

	if (fdt_magic(fdt) != FDT_MAGIC)
		return -FDT_ERR_BADMAGIC;
	if (!can_assume(LATEST)) {
		if ((fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
		    || (fdt_last_comp_version(fdt) >
			FDT_LAST_SUPPORTED_VERSION))
			return -FDT_ERR_BADVERSION;
		if (fdt_version(fdt) < fdt_last_comp_version(fdt))
			return -FDT_ERR_BADVERSION;
	}
	hdrsize = fdt_header_size(fdt);
	if (!can_assume(VALID_DTB)) {

		if ((fdt_totalsize(fdt) < hdrsize)
		    || (fdt_totalsize(fdt) > INT_MAX))
			return -FDT_ERR_TRUNCATED;

		/* Bounds check memrsv block */
		if (!check_off_(hdrsize, fdt_totalsize(fdt),
				fdt_off_mem_rsvmap(fdt)))
			return -FDT_ERR_TRUNCATED;
	}

	if (!can_assume(VALID_DTB)) {
		/* Bounds check structure block */
		if (!can_assume(LATEST) && fdt_version(fdt) < 17) {
			if (!check_off_(hdrsize, fdt_totalsize(fdt),
					fdt_off_dt_struct(fdt)))
				return -FDT_ERR_TRUNCATED;
		} else {
			if (!check_block_(hdrsize, fdt_totalsize(fdt),
					  fdt_off_dt_struct(fdt),
					  fdt_size_dt_struct(fdt)))
				return -FDT_ERR_TRUNCATED;
		}

		/* Bounds check strings block */
		if (!check_block_(hdrsize, fdt_totalsize(fdt),
				  fdt_off_dt_strings(fdt),
				  fdt_size_dt_strings(fdt)))
			return -FDT_ERR_TRUNCATED;
	}

	return 0;
}

const void *fdt_offset_ptr(const void *fdt, int offset, unsigned int len)
{
	unsigned int uoffset = offset;
	unsigned int absoffset = offset + fdt_off_dt_struct(fdt);

	if (offset < 0)
		return NULL;

	if (!can_assume(VALID_INPUT))
		if ((absoffset < uoffset)
		    || ((absoffset + len) < absoffset)
		    || (absoffset + len) > fdt_totalsize(fdt))
			return NULL;

	if (can_assume(LATEST) || fdt_version(fdt) >= 0x11)
		if (((uoffset + len) < uoffset)
		    || ((offset + len) > fdt_size_dt_struct(fdt)))
			return NULL;


	return fdt_offset_ptr_(fdt, offset);
}

uint32_t fdt_next_tag(const void *fdt, int startoffset, int *nextoffset)
{
	const fdt32_t *tagp, *lenp;
	uint32_t tag;
	int offset = startoffset;
	const char *p;

	*nextoffset = -FDT_ERR_TRUNCATED;
	tagp = fdt_offset_ptr(fdt, offset, FDT_TAGSIZE);
	if (!can_assume(VALID_DTB) && !tagp)
		return FDT_END; /* premature end */
	tag = fdt32_to_cpu(*tagp);
	//uk_pr_crit("fdt: %p, struct: %p, offset: %d, startoffset %d\n", fdt, fdt + fdt_off_dt_struct(fdt), fdt_off_dt_struct(fdt), offset);
	//uk_pr_crit("ntpt1, %d, %p\n", ((const char *)tagp+offset), ((const char *)tagp+offset));
	offset += FDT_TAGSIZE;
	//offset += 1;
	//uk_pr_crit("ntpt2, %d, %p\n", ((const char *)tagp+offset), ((const char *)tagp+offset));
	//uk_pr_crit("ntg, %s, tagp %p, manual %p\n", ((const char *)tagp+(uint32_t)1), ((const char *)tagp+offset), (const char *)fdt + fdt_off_dt_struct(fdt)+offset);
	*nextoffset = -FDT_ERR_BADSTRUCTURE;
	switch (tag) {
	case FDT_BEGIN_NODE:
		/* skip name */
		do {
		//	uk_pr_crit("bn1, %s, tagp %p, manual %p\n", ((const char *)tagp+offset), ((const char *)tagp+offset), (const char *)fdt + fdt_off_dt_struct(fdt)+offset);
			// p = fdt_offset_ptr(fdt, offset, 1);
			// uk_pr_crit("begin node %s\n",p);
			p = fdt_offset_ptr(fdt, offset++, 1);
			// for (int i = 0; i < 20; i++){
			// 	uk_pr_crit("char %d, %c\n", i, *(p+i));
			// }
		} while (p && (*p != '\0'));
		if (!can_assume(VALID_DTB) && !p) {
		//	uk_pr_crit("I don't understand\n");
			return FDT_END; /* premature end */
		}
		break;

	case FDT_PROP:
	//	uk_pr_crit("it's a prop\n");
		lenp = fdt_offset_ptr(fdt, offset, sizeof(*lenp));
		if (!can_assume(VALID_DTB) && !lenp)
			return FDT_END; /* premature end */
		/* skip-name offset, length and value */
		offset += 3*sizeof(fdt32_t) - FDT_TAGSIZE
			+ fdt32_to_cpu(*lenp);
		if (!can_assume(LATEST) &&
		    fdt_version(fdt) < 0x10 && fdt32_to_cpu(*lenp) >= 8 &&
		    ((offset - fdt32_to_cpu(*lenp)) % 8) != 0) {
			offset += 4;
		//	uk_pr_crit("was able to assume\n");
		}
		break;

	case FDT_END:
	case FDT_END_NODE:
	case FDT_NOP:
	//uk_pr_crit("nop tag\n");
		break;

	default:
//	uk_pr_crit("default, %d\n", tag);
		return FDT_END;
	}

	if (!fdt_offset_ptr(fdt, startoffset, offset - startoffset)) {
	//	uk_pr_crit("premature?\n");
		return FDT_END; /* premature end */
	}

	*nextoffset = FDT_TAGALIGN(offset);
	//uk_pr_crit("*nextoffset = %d\n", *nextoffset);
	return tag;
}

int fdt_check_node_offset_(const void *fdt, int offset)
{
	if (!can_assume(VALID_INPUT)
	    && ((offset < 0) || (offset % FDT_TAGSIZE)))
		return -FDT_ERR_BADOFFSET;

	if (fdt_next_tag(fdt, offset, &offset) != FDT_BEGIN_NODE)
		return -FDT_ERR_BADOFFSET;

//	uk_pr_crit("next offset %d\n", offset);
	return offset;
}

int fdt_check_prop_offset_(const void *fdt, int offset)
{
	if (!can_assume(VALID_INPUT)
	    && ((offset < 0) || (offset % FDT_TAGSIZE)))
		return -FDT_ERR_BADOFFSET;

	if (fdt_next_tag(fdt, offset, &offset) != FDT_PROP)
		return -FDT_ERR_BADOFFSET;

	return offset;
}

int fdt_next_node(const void *fdt, int offset, int *depth)
{
	int nextoffset = 0;
	uint32_t tag;

//	uk_pr_crit("next node offset : %d\n", offset);
	if (offset >= 0)
		if ((nextoffset = fdt_check_node_offset_(fdt, offset)) < 0)
			return nextoffset;
//	uk_pr_crit("passed the offset check\n");
	do {
		offset = nextoffset;
//		uk_pr_crit("offset = nextoffset %d\n", offset);
		tag = fdt_next_tag(fdt, offset, &nextoffset);
//		uk_pr_crit("tag %d\n", tag);

		switch (tag) {
		case FDT_PROP:
		case FDT_NOP:
			break;

		case FDT_BEGIN_NODE:
	//	uk_pr_crit("got a begin node here\n");
			if (depth)
				(*depth)++;
			break;

		case FDT_END_NODE:
			if (depth && ((--(*depth)) < 0))
				return nextoffset;
			break;

		case FDT_END:
			if ((nextoffset >= 0)
			    || ((nextoffset == -FDT_ERR_TRUNCATED) && !depth)) {
				uk_pr_crit("fdt err notfound\n");
				return -FDT_ERR_NOTFOUND;

			}
			else
				return nextoffset;
		}
	} while (tag != FDT_BEGIN_NODE);
//	uk_pr_crit("next node offset to return: %d\n", offset);
	return offset;
}

int fdt_first_subnode(const void *fdt, int offset)
{
	int depth = 0;

	offset = fdt_next_node(fdt, offset, &depth);
	if (offset < 0 || depth != 1)
		return -FDT_ERR_NOTFOUND;

	return offset;
}

int fdt_next_subnode(const void *fdt, int offset)
{
	int depth = 1;

	/*
	 * With respect to the parent, the depth of the next subnode will be
	 * the same as the last.
	 */
	do {
		offset = fdt_next_node(fdt, offset, &depth);
		if (offset < 0 || depth < 1)
			return -FDT_ERR_NOTFOUND;
	} while (depth > 1);

	return offset;
}

const char *fdt_find_string_(const char *strtab, int tabsize, const char *s)
{
	int len = strlen(s) + 1;
	const char *last = strtab + tabsize - len;
	const char *p;

	for (p = strtab; p <= last; p++)
		if (memcmp(p, s, len) == 0)
			return p;
	return NULL;
}

int fdt_move(const void *fdt, void *buf, int bufsize)
{
	if (!can_assume(VALID_INPUT) && bufsize < 0)
		return -FDT_ERR_NOSPACE;

	FDT_RO_PROBE(fdt);

	if (fdt_totalsize(fdt) > (unsigned int)bufsize)
		return -FDT_ERR_NOSPACE;

	memmove(buf, fdt, fdt_totalsize(fdt));
	return 0;
}
