/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Stefan Teodorescu <stefanl.teodorescu@gmail.com>
 *
 * Copyright (c) 2021, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __UKPLAT_MM__
#define __UKPLAT_MM__

#include <sys/types.h>
#include <uk/config.h>

#ifndef CONFIG_PT_API
#error Using this header requires enabling the virtual memory management API
#endif /* CONFIG_PT_API */

#define MEM_REGIONS_NUMBER	4

struct phys_mem_region
{
	unsigned long start_addr;
	size_t length;

	unsigned long bitmap_start_addr;
	size_t bitmap_length;

	// TODO
	size_t last_offset;
};

extern size_t _phys_mem_region_list_size;
extern struct phys_mem_region _phys_mem_region_list[MEM_REGIONS_NUMBER];

#ifdef CONFIG_PARAVIRT
#include <uk/asm/mm_pv.h>
#else
//#include <uk/asm/mm_native.h>
#endif	/* CONFIG_PARAVIRT */

#define PTE_ATTR_UXN			1 << 54
#define PTE_ATTR_PXN			1 << 53
#define PTE_ATTR_XN			(PTE_ATTR_PXN | PTE_ATTR_UXN)
#define PTE_ATTR_CONTIGUOUS		1 << 52
#define PTE_ATTR_DBM			1 << 51
#define PTE_ATTR_GP			(1 << 50)
#define PTE_ATTR_nG			(1 << 11)
#define PTE_ATTR_AF			(1 << 10)

#define DEVICE_nGnRnE	0
#define DEVICE_nGnRE	1
#define DEVICE_GRE	2
#define NORMAL_NC	3
#define NORMAL_WT	4
#define NORMAL_WB	5

#define PTE_ATTR_SH(x)			((x) << 8)
#define PTE_ATTR_SH_MASK		PTE_ATTR_SH(3)
#define PTE_ATTR_SH_NS			0 /* Non-shareable */
#define PTE_ATTR_SH_OS			2 /* Outer-shareable */
#define PTE_ATTR_SH_IS			3 /* Inner-shareable */
#define PTE_ATTR_AP_RW_BIT		(1 << 7)
#define PTE_ATTR_AP(x)			((x) << 6)
#define PTE_ATTR_AP_MASK		ATTR_AP(3)
#define PTE_ATTR_AP_RW			PTE_ATTR_AP(0 << 1)
#define PTE_ATTR_AP_RO			PTE_ATTR_AP((1 << 1))
#define PTE_ATTR_AP_USER		(1 << 0)
#define PTE_ATTR_NS			(1 << 5)
#define PTE_ATTR_IDX(x)			((x) << 2)
#define PTE_ATTR_IDX_MASK		(7 << 2)

/* Memory attributes */
#define PTE_ATTR_DEFAULT					\
	(PTE_ATTR_AF | PTE_ATTR_SH(PTE_ATTR_SH_IS))

#define PTE_ATTR_DEVICE_nGnRE					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_XN | PTE_ATTR_IDX(DEVICE_nGnRE))

#define PTE_ATTR_DEVICE_nGnRnE					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_XN | PTE_ATTR_IDX(DEVICE_nGnRnE))

#ifdef CONFIG_ARM64_FEAT_MTE
#define PTE_ATTR_NORMAL_RW					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_XN | PTE_ATTR_IDX(NORMAL_WB_TAGGED))
#else
#define PTE_ATTR_NORMAL_RW					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_XN | PTE_ATTR_IDX(NORMAL_WB))
#endif /* CONFIG_ARM64_FEAT_MTE */

#define PTE_ATTR_NORMAL_RO					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_XN |			\
	 PTE_ATTR_IDX(NORMAL_WB) | PTE_ATTR_AP_RW_BIT)

#define PTE_ATTR_NORMAL_RWX					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_UXN | PTE_ATTR_IDX(NORMAL_WB))
#define PTE_ATTR_NORMAL_RX					\
	(PTE_ATTR_DEFAULT | PTE_ATTR_UXN |			\
	 PTE_ATTR_IDX(NORMAL_WB) | PTE_ATTR_AP_RW_BIT)

#define PAGE_ALIGN_UP(vaddr)		ALIGN_UP(vaddr, PAGE_SIZE)

#define PAGE_ALIGNED(vaddr)		(!((vaddr) & (PAGE_SIZE - 1)))
#define PAGE_ALIGN_DOWN(vaddr)		ALIGN_DOWN(vaddr, PAGE_SIZE)

#define PT_LEVELS			4
#define PT_PTES_PER_LEVEL		512
#define PT_LEVEL_SHIFT			9

/* We use plain values here so we do not create dependencies on external helper
 * macros, which would forbid us to use the macros in functions defined further
 * down in this header.
 */
#define PAGE_LEVEL			0
#define PAGE_SHIFT			12
#define PAGE_SIZE			0x1000UL
#define PAGE_MASK			(~(PAGE_SIZE - 1))

#define PAGE_ATTR_PROT_MASK		0x0f
#define PAGE_ATTR_PROT_SHIFT		0

#define PAGE_ATTR_PROT_NONE		0x00
#define PAGE_ATTR_PROT_READ		0x01
#define PAGE_ATTR_PROT_WRITE		0x02
#define PAGE_ATTR_PROT_EXEC		0x04

#define PAGE_ATTR_TYPE_MASK		0x07
#define PAGE_ATTR_TYPE_SHIFT		5

#define PAGE_ATTR_TYPE_NORMAL_WB	(0 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_NORMAL_WT	(1 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_NORMAL_NC	(2 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_DEVICE_nGnRnE	(3 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_DEVICE_nGnRE	(4 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_DEVICE_GRE	(5 << PAGE_ATTR_TYPE_SHIFT)
#define PAGE_ATTR_TYPE_NORMAL_WB_TAGGED	(6 << PAGE_ATTR_TYPE_SHIFT)

#define PAGE_ATTR_SHAREABLE_MASK	0x03
#define PAGE_ATTR_SHAREABLE_SHIFT	8

#define PAGE_ATTR_SHAREABLE_NS		(0 << PAGE_ATTR_SHAREABLE_SHIFT)
#define PAGE_ATTR_SHAREABLE_IS		(1 << PAGE_ATTR_SHAREABLE_SHIFT)
#define PAGE_ATTR_SHAREABLE_OS		(2 << PAGE_ATTR_SHAREABLE_SHIFT)

/* Page fault error code bits */
#define ARM64_PF_ESR_WnR		0x0000040UL
#define ARM64_PF_ESR_ISV		0x1000000UL

#define ARM64_PADDR_BITS		48
#define ARM64_VADDR_BITS		48

#define PTE_TYPE_PAGE 0x3

#define PTE_BLOCK_DEVICE_nGnRnE	(PTE_ATTR_DEVICE_nGnRnE + PTE_TYPE_BLOCK)
#define PTE_BLOCK_DEVICE_nGnRE	(PTE_ATTR_DEVICE_nGnRE + PTE_TYPE_BLOCK)
#define PTE_BLOCK_NORMAL_RW	(PTE_ATTR_NORMAL_RW + PTE_TYPE_BLOCK)
#define PTE_BLOCK_NORMAL_RWX	(PTE_ATTR_NORMAL_RWX + PTE_TYPE_BLOCK)
#define PTE_PAGE_NORMAL_RO	(PTE_ATTR_NORMAL_RO  + PTE_TYPE_PAGE)
#define PTE_PAGE_NORMAL_RWX	(PTE_ATTR_NORMAL_RWX  + PTE_TYPE_PAGE)

/**
 * Get a free frame in the physical memory where a new mapping can be created.
 *
 * @param flags: specify any criteria that the frame has to meet (e.g. a 2MB
 * frame for a large page). These are constructed by or'ing PAGE_FLAG_* flags.
 *
 * @return: physical address of an unused frame or PAGE_INVALID on failure.
 */
// static inline unsigned long uk_get_next_free_frame(unsigned long flags)
// {
// 	size_t i;
// 	unsigned long offset;
// 	unsigned long pfn;
// 	unsigned long frame_size;

// 	unsigned long phys_bitmap_start_addr;
// 	size_t phys_bitmap_length;

// 	unsigned long phys_mem_start_addr;
// 	size_t phys_mem_length;

// 	size_t last_offset;

// #ifdef CONFIG_PARAVIRT
// 	/*
// 	 * Large/Huge pages are not supported in PV guests on Xen.
// 	 * https://wiki.xenproject.org/wiki/Huge_Page_Support
// 	 */
// 	if (flags & PAGE_FLAG_LARGE) {
// 		uk_pr_err("Large pages are not supported on PV guest\n");
// 		return PAGE_INVALID;
// 	}
// #endif /* CONFIG_PARAVIRT */

// 	if (flags & PAGE_FLAG_LARGE)
// 		frame_size = PAGE_LARGE_SIZE >> PAGE_SHIFT;
// 	else
// 		frame_size = 1;

// 	for (i = 0; i < _phys_mem_region_list_size; i++) {
// 		phys_mem_start_addr =
// 			_phys_mem_region_list[i].start_addr;
// 		phys_mem_length =
// 			_phys_mem_region_list[i].length;
// 		phys_bitmap_start_addr =
// 			_phys_mem_region_list[i].bitmap_start_addr;
// 		phys_bitmap_length =
// 			_phys_mem_region_list[i].bitmap_length;
// 		last_offset =
// 			_phys_mem_region_list[i].last_offset;

// 		if (phys_bitmap_length - last_offset <= 1) {
// 			last_offset = 0;
// 			_phys_mem_region_list[i].last_offset = 0;
// 		}

// 		offset = uk_bitmap_find_next_zero_area(
// 				(unsigned long *) phys_bitmap_start_addr,
// 				phys_bitmap_length,
// 				last_offset /* start */,
// 				frame_size /* nr */,
// 				frame_size - 1 /* align_mask */);

// 		if (offset * PAGE_SIZE > phys_mem_length)
// 			continue;

// 		uk_bitmap_set((unsigned long *) phys_bitmap_start_addr, offset,
// 			      frame_size);
// 		_phys_mem_region_list[i].last_offset = offset + frame_size - 1;

// 		pfn = (phys_mem_start_addr >> PAGE_SHIFT) + offset;

// 		return pfn_to_mframe(pfn);
// 	}

// 	uk_pr_err("Out of physical memory\n");
// 	return PAGE_INVALID;
// }

// static inline int uk_frame_reserve(unsigned long paddr, unsigned long frame_size,
// 		int val)
// {
// 	size_t i;
// 	unsigned long offset;

// 	unsigned long bitmap_start_addr;

// 	unsigned long mem_start_addr;
// 	unsigned long mem_length;

// 	/* TODO: add huge pages */
// 	if (frame_size != PAGE_SIZE && frame_size != PAGE_LARGE_SIZE)
// 		return -1;

// 	if (paddr & (frame_size - 1))
// 		return -1;

// 	frame_size >>= PAGE_SHIFT;

// 	for (i = 0; i < _phys_mem_region_list_size; i++) {
// 		mem_start_addr =
// 			_phys_mem_region_list[i].start_addr;
// 		mem_length =
// 			_phys_mem_region_list[i].length;
// 		bitmap_start_addr =
// 			_phys_mem_region_list[i].bitmap_start_addr;

// 		if (!IN_RANGE(paddr, mem_start_addr, mem_length))
// 			continue;

// 		offset = (paddr - mem_start_addr) >> PAGE_SHIFT;
// 		if (val) {
// 			uk_bitmap_set((unsigned long *) bitmap_start_addr,
// 				offset, frame_size);
// 		} else {
// 			uk_bitmap_clear((unsigned long *) bitmap_start_addr,
// 				offset, frame_size);
// 		}
// 		return 0;
// 	}

// 	return -1;
// }

/**
 * Create a mapping from a virtual address to a physical address, with given
 * protections and flags.
 *
 * @param vaddr: the virtual address of the page that is to be mapped.
 * @param paddr: the physical address of the frame to which the virtual page
 * is mapped to. This parameter can be equal to PAGE_PADDR_ANY when the caller
 * is not interested in the physical address where the mapping is created.
 * @param prot: protection permissions of the page (obtained by or'ing
 * PAGE_PROT_* flags).
 * @param flags: flags of the page (obtained by or'ing PAGE_FLAG_* flags).
 *
 * @return: 0 on success and -1 on failure. The uk_page_map call can fail if:
 * - the given physical or virtual addresses are not aligned to page size;
 * - any page in the region is already mapped to another frame;
 * - if PAGE_PADDR_ANY flag is selected and there are no more available
 *   free frames in the physical memory;
 * - (on Xen PV) if flags contains PAGE_FLAG_LARGE - large pages are not
 *   supported on PV guests;
 * - (on Xen PV) the hypervisor rejected the mapping.
 *
 * In case of failure, the mapping is not created.
 */
int uk_page_map(unsigned long vaddr, unsigned long paddr, unsigned long prot,
		unsigned long flags);

/**
 * Create a mapping from a region starting at a virtual address to a physical
 * address, with given protections and flags.
 *
 * @param vaddr: the virtual address of the page where the region that is to be
 * mapped starts.
 * @param paddr: the physical address of the starting frame of the region to
 * which the virtual region is mapped to. This parameter can be equal to
 * PAGE_PADDR_ANY when the caller is not interested in the physical address
 * where the mappings are created.
 * @param prot: protection permissions of the pages (obtained by or'ing
 * PAGE_PROT_* flags).
 * @param flags: flags of the page (obtained by or'ing PAGE_FLAG_* flags).
 *
 * @return: 0 on success and -1 on failure. The uk_page_map call can fail if:
 * - the given physical or virtual addresses are not aligned to page size;
 * - any page in the region is already mapped to another frame;
 * - if PAGE_PADDR_ANY flag is selected and there are no more available
 *   free frames in the physical memory;
 * - (on Xen PV) if flags contains PAGE_FLAG_LARGE - large pages are not
 *   supported on PV guests;
 * - (on Xen PV) the hypervisor rejected any of the mappings.
 *
 * In case of failure, no new mapping is created.
 */
int uk_map_region(unsigned long vaddr, unsigned long paddr,
		unsigned long pages, unsigned long prot, unsigned long flags);

/**
 * Frees a mapping for a page.
 *
 * @param vaddr: the virtual address of the page that is to be unmapped.
 *
 * @return: 0 in case of success and -1 on failure. The call fails if:
 * - the given page is not mapped to any frame;
 * - the virtual address given is not aligned to page (simple/large/huge) size.
 * - (on Xen PV) the hypervisor rejected the unmapping.
 */
int uk_page_unmap(unsigned long vaddr);

unsigned long get_pte(unsigned long vaddr);

unsigned long* pt_pte_to_cap(unsigned long pte, int level);

int uk_copy_ptes(unsigned long src_start_vaddr, unsigned long src_end_vaddr, unsigned long dst_start_vaddr);
int uk_pages_set_prot(unsigned long vaddr_start, unsigned long pages,
	unsigned long new_prot);
	int uk_pages_set_prot_nocopa(unsigned long vaddr_start, unsigned long pages,
	unsigned long new_prot);

/**
 * Sets new protections for a given page.
 *
 * @param vaddr: the virtual address of the page whose protections are updated.
 * @param new_prot: new protections that will be set to the page (obtained by
 * or'ing PAGE_PROT_* flags).
 *
 * @return: 0 in case of success and -1 on failure. The call fails if:
 * - the given page is not mapped to any frame;
 * - the virtual address given is not aligned to page (simple/large/huge) size.
 * - (on Xen PV) the hypervisor rejected the unmapping.
 */
int uk_page_set_prot(unsigned long vaddr, unsigned long new_prot);

/**
 * Return page table entry corresponding to given virtual address.
 * @param vaddr: the virtual address, aligned to the corresponding page
 * dimesion (simple, large or huge) size.
 * @return: page table entry (PTE) obtained by doing a page table walk.
 */
unsigned long uk_virt_to_pte(unsigned long vaddr);

/**
 * Initialize internal page table bookkeeping for using the PT API when
 * attaching to an existing page table.
 * @param pt_area_start: the virtual address of the area for page tables and
 * internal bookkeeping.
 * @param paddr_start: the physical address of the beginning of the area that
 * should be managed by the API.
 * @param len: the length of the (physical) memory area that should be managed.
 */
void uk_pt_init(unsigned long pt_area_start, unsigned long paddr_start,
		size_t len);

/**
 * TODO: params
 */
int uk_pt_add_mem(unsigned long paddr_start, unsigned long len);

/**
 * Build page table structure from scratch
 * @param paddr_start: the first address in the usable physical memory.
 * @param len: the length (in bytes) of the physical memory that will be
 * managed by the API.
 * TODO params
 *
 * This function builds a structure of page tables (by calling _pt_create),
 * initializes the page table API (by calling uk_pt_init), maps the kernel in
 * the virtual address space (with _mmap_kernel), switches to the new address
 * space and sets the _virt_offset variable.
 */
void uk_pt_build(unsigned long paddr_start, unsigned long len,
		unsigned long kernel_start_vaddr,
		unsigned long kernel_start_paddr,
		unsigned long kernel_area_size);

/**
 * Allocate a new stack and return address to its lower address.
 *
 * @return: the lower address of the stack. If the returned address is `addr`,
 * then the allocated stack region is [`addr`, `addr + __STACK_SIZE`]. The
 * maximum number of stacks that can be allocated denotes the maximum number
 * of threads that can co-exist. More details about the number of stacks in
 * include/uk/mem_layout.h. Returns NULL in case of failure.
 */
void *uk_stack_alloc();

/**
 * Frees a stack previously allocated with uk_stack_alloc().
 *
 * @param vaddr: the virtual address of the beginning of the stack (i.e. the
 * address returned by uk_stack_alloc()).
 *
 * @return: 0 in case of success and -1 on failure. The call can fail if:
 * - the given address is not a stack address previously returned by
 *   uk_stack_alloc (which is between STACK_AREA_BEGIN and STACK_AREA_END);
 * - the given address is not page aligned;
 * - (on Xen) the hypervisor rejected the unmapping.
 */
int uk_stack_free(void *vaddr);

/**
 * Create virtual mappings for a new heap of a given length at a given virtual
 * address.
 *
 * @param vaddr: the virtual address of the beginning of the area where the
 * heap will be mapped.
 * @param len: the length (in bytes) of the heap.
 *
 * @return: 0 in case of success and -1 on failure. The call can fail if:
 * - the given interval [vaddr, vaddr + len] is not contained in the interval
 *   [HEAP_AREA_BEGIN, HEAP_AREA_END];
 * - uk_mmap_region fails.
 */
int uk_heap_map(unsigned long vaddr, unsigned long len);

#endif /* __UKPLAT_MM__ */

