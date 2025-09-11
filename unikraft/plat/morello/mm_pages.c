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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <uk/assert.h>
#include <uk/list.h>
#include <uk/print.h>
#include <morello/mm.h>
#include <uk/plat/mm.h>
#include <uk/mem_layout.h>
#include <uk/arch/types.h>
#include <sfork.h>
#include <flexos/isolation.h>
#include <morello/setup.h>

#define L1_SHIFT 30
#define L2_SHIFT 21
#define L3_SHIFT 12

#define PT_INDEX_MASK 0x1FF
#define PTE_LOWER_ATTR_MASK 0x7F8

#define L1_OFFSET(vaddr) (vaddr >> L1_SHIFT) & PT_INDEX_MASK
#define L2_OFFSET(vaddr) (vaddr >> L2_SHIFT) & PT_INDEX_MASK
#define L3_OFFSET(vaddr) (vaddr >> L3_SHIFT) & PT_INDEX_MASK

#define PTE_L1_BLOCK_PADDR_MASK		0x0000ffffc0000000
#define PTE_L2_BLOCK_PADDR_MASK		0x0000ffffffe00000
#define PTE_L3_PAGE_PADDR_MASK		0x0000fffffffff000
#define PTE_Lx_TABLE_PADDR_MASK 0x0000fffffffff000

#define L3_PT_ENTRY_TEMPLATE 0x1000000000000007

static unsigned long pt_bitmap_start_addr;
static unsigned long pt_bitmap_length;

static unsigned long pt_mem_start_addr;
static unsigned long pt_mem_length;

// static unsigned long stack_bitmap_start_addr[UK_BITS_TO_LONGS(STACK_COUNT)];
// static unsigned long stack_bitmap_length = UK_BITS_TO_LONGS(STACK_COUNT);

// size_t _phys_mem_region_list_size;
// struct phys_mem_region _phys_mem_region_list[MEM_REGIONS_NUMBER];


static inline void ukarch_tlb_flush_entry(unsigned long vaddr)
{
	__u64 page_number = vaddr >> PAGE_SHIFT;
	__asm__ __volatile__(
		"	dsb	ishst\n"        /* wait for write complete */
		"	tlbi	vaae1is, %x0\n" /* invalidate by vaddr */
		"DC CVAU, %1\n"
		"	dsb	ish\n"          /* wait for invalidate compl */
		"	isb\n"                  /* sync context */
		:: "r" (page_number), "r"(int_to_cap(vaddr)) : "memory");
}

static inline void ukarch_tlb_flush(void)
{
	__asm__ __volatile__(
			"	dsb	ishst\n"     /* wait for write complete */
		"	tlbi	vmalle1is\n" /* invalidate all */
        "TLBI VMALLE1\n"
		"	dsb	ish\n"       /* wait for invalidate complete */
        "   dsb st\n"
		"	dsb sy\n"
		"	isb\n"               /* sync context */
		::: "memory");
}

static unsigned long prot_to_pte(int prot)
{
	unsigned long page_prot = 0;
	// if (prot & PAGE_ATTR_PROT_NONE)
	// 	page_prot = page_prot;
	if (prot == PAGE_ATTR_PROT_NONE)
		page_prot = 0x2000000000000000;
	else if (prot & PAGE_ATTR_PROT_READ)
		//page_prot = ((unsigned long)PTE_ATTR_AP_RO | (unsigned long)(0x2 << 61));
		page_prot = 0x4000000000000480;
	else if (prot & PAGE_ATTR_PROT_WRITE)
		//page_prot |= (unsigned long)PTE_ATTR_AP_RW | (unsigned long)(0x1 << 61);
		page_prot = 0x2000000000000400;
	else if (prot & PAGE_ATTR_PROT_EXEC)
		page_prot |= (unsigned long)PAGE_ATTR_PROT_EXEC | (unsigned long)(0x1 << 61);

	//uk_pr_crit("New prot pte %lx\n", page_prot);

	return page_prot;
}

inline static unsigned long clear_pte_perms(unsigned long pte) {
	unsigned long pte1 = pte;
	uk_pr_crit("pte %ld\n", pte);
	asm("mov x1, %1\n"
	"ldr x0, [%0]\n"
	"bic x2, x0, x1\n"
	"str x2, [%0]\n" : : "r"(&pte1), "r"(PTE_LOWER_ATTR_MASK) : "x0", "x1", "x2");
	//uk_pr_crit("pte1 %ld\n", pte1);
	return pte1;
}


unsigned long _virt_offset;

unsigned long* ukarch_read_pt_base(void)
{

//#if defined CONFIG_LIBSASSYFORK_SMP || CONFIG_LIBSASSYFORK_COA
    return int_to_cap(pt_base);
// #else
// 	unsigned long* cbase;
// //potential issue here, but we'll get to that
// 	asm("mrs x0, ttbr0_el1\n"
// 		"cvtd c0, x0\n"
// 		"str c0, [%0]\n" : : "r"(&cbase): "c0");
// //		uk_pr_crit("base: %p\n", pt_base);
// //		UK_ASSERT(__builtin_cheri_tag_get(cbase) == 1);
// 	return cbase;
// #endif
}

// Note that this is assuming a 1-1 mapping
unsigned long* pt_pte_to_cap(unsigned long pte, int level) {
	int block_or_table = pte & 0x3;
	unsigned long addr;
	unsigned long* vaddr;
	switch (level) {
		case 1:
		case 2:
			if (block_or_table == 0x3) { //is table
				addr = pte & PTE_Lx_TABLE_PADDR_MASK;
				//uk_pr_crit("pte addr read %p\n", addr);
			} else {
				return 1;
			}
			break;

		case 3:
			if (block_or_table == 0x3) { //is page yay
				addr = pte & PTE_L3_PAGE_PADDR_MASK;
			}else {
				return 1;
			}
			break;
	}
	// asm ("cvtd c0, %0 \n"
	// 	"str c0, [%1]\n" : : "r"(addr), "r"(&vaddr) : "c0");
	// TODO this so needs to be fixed 
	vaddr = int_to_cap(addr);
	return vaddr;

}

static inline int ukarch_pte_write(unsigned long* pt, size_t offset,
		unsigned long val, size_t level)
{
	//UK_ASSERT(level >= 1 && level <= PAGETABLE_LEVELS);
	//UK_ASSERT(PAGE_ALIGNED(pt));
	//UK_ASSERT(offset < pagetable_entries[level - 1]);

	//UK_ASSERT(__builtin_cheri_tag_get(pt+offset) == 1);
//	uk_pr_crit("Perms %x\n", __builtin_cheri_perms_get(pt+offset));
	//UK_ASSERT(__builtin_cheri_perms_get(pt+offset)&__CHERI_CAP_PERMISSION_PERMIT_LOAD__ == 1);
#ifdef CONFIG_LIBSASSYFORK_SMP
__builtin___clear_cache(pt+offset, pt+offset+ADVANCE_CAPABILITY(PAGE_SIZE));
#endif
	*(pt + offset) = val;

	return 0;
}

static inline unsigned long ukarch_pte_read(unsigned long* pt, size_t offset,
		size_t level)
{
#ifdef CONFIG_LIBSASSYFORK_SMP
__builtin___clear_cache(pt+offset, pt+offset+ADVANCE_CAPABILITY(PAGE_SIZE));
#endif
	return *(pt + offset);
}

static inline unsigned long* ukarch_pt_get_precise(unsigned long* pt, size_t offset)
{

	return (pt + offset);
}

/*
 * Variable used in the initialization phase during booting when allocating
 * page tables does not use the page table API function uk_pt_alloc_table.
 * The initial page tables are allocated sequantially and this variable is the
 * counter of used page tables.
 */
static size_t _used_pts_count;

/* TODO fix duplicate with POSIX mmap */
static unsigned long get_free_virtual_area(unsigned long start, size_t length,
		unsigned long interval_end)
{
	unsigned long page;

	if (!PAGE_ALIGNED(length))
		return -1;

	while (start <= interval_end - length) {
		for (page = start; page < start + length; page += PAGE_SIZE) {
			if (PAGE_PRESENT(uk_virt_to_pte(page)))
				break;
		}

		if (page == start + length)
			return start;

		start = page + PAGE_SIZE;
	}

	return -1;
}

/**
 * Allocate a page table for a given level (in the PT hierarchy).
 *
 * @param level: the level of the needed page table.
 *
 * @return: virtual address of newly allocated page table or PAGE_INVALID
 * on failure.
 */
// static unsigned long uk_pt_alloc_table(size_t level, int is_initmem)
// {
// 	unsigned long offset, pt_vaddr;
// #ifdef CONFIG_PARAVIRT
// 	int rc;
// #endif	/* CONFIG_PARAVIRT */

// 	offset = uk_bitmap_find_next_zero_area(
// 	    (unsigned long *) pt_bitmap_start_addr,
// 	    pt_bitmap_length,
// 	    0 /* start */, 1 /* nr */, 0 /* align_mask */);

// 	if (offset * PAGE_SIZE > pt_mem_length) {
// 		uk_pr_err("Filled up all available space for page tables\n");
// 		return PAGE_INVALID;
// 	}

// 	uk_bitmap_set((unsigned long *) pt_bitmap_start_addr, offset, 1);
// 	pt_vaddr = pt_mem_start_addr + (offset << PAGE_SHIFT) + _virt_offset;

// #ifdef CONFIG_PARAVIRT
// 	rc = uk_page_set_prot(pt_vaddr, PAGE_PROT_READ | PAGE_PROT_WRITE);
// 	if (rc)
// 		return PAGE_INVALID;
// #endif	/* CONFIG_PARAVIRT */

// 	memset((void *) pt_vaddr, 0,
// 		sizeof(unsigned long) * pagetable_entries[level - 1]);

// 	/* Xen requires that PTs are mapped read-only */
// #ifdef CONFIG_PARAVIRT
// 	/*
// 	 * When using this function on Xen for the initmem part, the page
// 	 * must not be set to read-only, as we are currently writing
// 	 * directly into it. All page tables will be set later to read-only
// 	 * before setting the new pt_base.
// 	 */
// 	if (!is_initmem) {
// 		rc = uk_page_set_prot(pt_vaddr, PAGE_PROT_READ);
// 		if (rc)
// 			return PAGE_INVALID;
// 	}
// #endif	/* CONFIG_PARAVIRT */

// 	/*
// 	 * This is an L(n + 1) entry, so we set L(n + 1) flags
// 	 * (Index in pagetable_protections is level of PT - 1)
// 	 */
// 	return (pt_virt_to_mfn(pt_vaddr) << PAGE_SHIFT)
// 		| pagetable_protections[level];
// }

// static int uk_pt_release_if_unused(unsigned long vaddr, unsigned long pt,
// 	unsigned long parent_pt, size_t level)
// {
// 	unsigned long offset;
// 	size_t i;
// 	int rc;

// 	if (!PAGE_ALIGNED(pt) || !PAGE_ALIGNED(parent_pt)) {
// 		uk_pr_err("Table's address must be aligned to page size\n");
// 		return -1;
// 	}

// 	for (i = 0; i < pagetable_entries[level - 1]; i++) {
// 		if (PAGE_PRESENT(ukarch_pte_read(pt, i, level)))
// 			return 0;
// 	}

// 	rc = ukarch_pte_write(parent_pt, Lx_OFFSET(vaddr, level + 1), 0,
// 		level + 1);
// 	if (rc)
// 		return -1;

// 	ukarch_flush_tlb_entry(parent_pt);

// 	offset = (pt - pt_mem_start_addr - _virt_offset) >> PAGE_SHIFT;
// 	uk_bitmap_clear((unsigned long *) pt_bitmap_start_addr, offset, 1);

// 	return 0;
// }

unsigned long* get_pt(unsigned long vaddr) {
	//unsigned long pvaddr = PAGE_ALIGN_DOWN(vaddr);
// 		#ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
//ukarch_tlb_flush();

	unsigned long* pt = ukarch_read_pt_base();

	unsigned long pte = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);
	//uk_pr_crit("vaddr %p, pte %x\n", vaddr, pte);
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 1);
	UK_ASSERT(__builtin_cheri_tag_get(pt) == 1);
	pte = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 2);
// 		#ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif

	return ukarch_pt_get_precise(pt, L3_OFFSET(vaddr));
}

unsigned long get_pte(unsigned long vaddr) {
	//unsigned long pvaddr = PAGE_ALIGN_DOWN(vaddr);
	#ifdef CONFIG_LIBSASSYFORK_SMP
	ukarch_spin_lock(&lock2);
#endif

	unsigned long* pt = ukarch_read_pt_base();

	unsigned long pte = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 1);
	UK_ASSERT(__builtin_cheri_tag_get(pt)== 1);
	pte = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 2);

	pte = ukarch_pte_read(pt, L3_OFFSET(vaddr), 3);
	#ifdef CONFIG_LIBSASSYFORK_SMP
	ukarch_spin_unlock(&lock2);
#endif
	return pte;
}

static int _page_map(unsigned long* pt, unsigned long vaddr, unsigned long paddr,
	  unsigned long prot, unsigned long flags, int is_initmem,
	  int (*pte_write)(unsigned long*, size_t, unsigned long, size_t))
{
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
	
	unsigned long pte;
	int rc;

	if (!PAGE_ALIGNED(vaddr)) {
	 	uk_pr_err("Virt address must be aligned to page size\n");
	 	return -1;
	}
	
	UK_ASSERT(__builtin_cheri_tag_get(pt) ==1);

	//uk_pr_crit("Mapping vaddr %p, paddr %p\n", vaddr, paddr);

	pte = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);
	//uk_pr_crit("Got first level pte\n");
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 1);
	UK_ASSERT(__builtin_cheri_tag_get(pt) ==1);
	pte = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
	//uk_pr_crit("Got second level pte\n");
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 2);

	unsigned long new_pte = L3_PT_ENTRY_TEMPLATE;
	new_pte |= paddr;
#ifdef CONFIG_LIBSASSYFORK_SMP
	new_pte |= prot_to_pte(prot) | PTE_ATTR_SH(PTE_ATTR_SH_IS);
#else
	new_pte |= prot_to_pte(prot) | PTE_ATTR_SH(PTE_ATTR_SH_OS);
#endif

	pte = ukarch_pte_read(pt, L3_OFFSET(vaddr), 3);
	//uk_pr_crit("Got third level pte\n");

	ukarch_pte_write(pt, L3_OFFSET(vaddr), new_pte, 3);
	ukarch_tlb_flush_entry(vaddr);
	//ukarch_tlb_flush();
	//uk_pr_crit("Stored new entry level pte\n");
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif
	
	// }

	return 0;
}

int _initmem_page_map(unsigned long pt, unsigned long vaddr,
		unsigned long paddr, unsigned long prot,
		unsigned long flags)
{
// 	return _page_map(pt, vaddr, paddr, prot, flags, 1,
// 		_ukarch_pte_write_raw);
 }

int uk_page_map(unsigned long vaddr, unsigned long paddr, unsigned long prot,
	unsigned long flags)
{
	//uk_pr_crit("Came from %p\n", LR());
	return _page_map(ukarch_read_pt_base(), vaddr, paddr, prot, flags, 0,
		ukarch_pte_write);
}

static int _page_unmap(unsigned long pt, unsigned long vaddr,
	int (*pte_write)(uintptr_t, size_t, unsigned long, size_t))
{
	// unsigned long l1_table, l2_table, l3_table, l4_table, pte;
	// unsigned long pfn;
	// unsigned long frame_size = PAGE_SIZE;
	// int rc;

	// if (!PAGE_ALIGNED(vaddr)) {
	// 	uk_pr_err("Address must be aligned to page size\n");
	// 	return -1;
	// }

	// l4_table = pt;
	// pte = ukarch_pte_read(l4_table, L4_OFFSET(vaddr), 4);
	// if (!PAGE_PRESENT(pte))
	// 	return -1;

	// l3_table = (unsigned long) pt_pte_to_virt(pte);
	// pte = ukarch_pte_read(l3_table, L3_OFFSET(vaddr), 3);
	// if (!PAGE_PRESENT(pte))
	// 	return -1;

	// l2_table = (unsigned long) pt_pte_to_virt(pte);
	// pte = ukarch_pte_read(l2_table, L2_OFFSET(vaddr), 2);
	// if (!PAGE_PRESENT(pte))
	// 	return -1;
	// if (PAGE_LARGE(pte)) {
	// 	if (!PAGE_LARGE_ALIGNED(vaddr))
	// 		return -1;

	// 	pfn = pte_to_pfn(pte);
	// 	rc = pte_write(l2_table, L2_OFFSET(vaddr), 0, 2);
	// 	if (rc)
	// 		return -1;
	// 	frame_size = PAGE_LARGE_SIZE;
	// } else {
	// 	l1_table = (unsigned long) pt_pte_to_virt(pte);
	// 	pte = ukarch_pte_read(l1_table, L1_OFFSET(vaddr), 1);
	// 	if (!PAGE_PRESENT(pte))
	// 		return -1;

	// 	pfn = pte_to_pfn(pte);
	// 	rc = pte_write(l1_table, L1_OFFSET(vaddr), 0, 1);
	// 	if (rc)
	// 		return -1;
	// 	rc = uk_pt_release_if_unused(vaddr, l1_table, l2_table, 1);
	// 	if (rc)
	// 		return -1;
	// 	}

	// ukarch_flush_tlb_entry(vaddr);

	// uk_frame_reserve(pfn << PAGE_SHIFT, frame_size, 0);

	// rc = uk_pt_release_if_unused(vaddr, l2_table, l3_table, 2);
	// if (rc)
	// 	return -1;
	// rc = uk_pt_release_if_unused(vaddr, l3_table, l4_table, 3);
	// if (rc)
	// 	return -1;

	return 0;
}

int uk_page_unmap(unsigned long vaddr)
{
	return _page_unmap(ukarch_read_pt_base(), vaddr, ukarch_pte_write);
}

static int _map_region(unsigned long* pt, unsigned long vaddr,
	unsigned long paddr, unsigned long pages, unsigned long prot,
	unsigned long flags, int is_initmem,
	int (*pte_write)(unsigned long*, size_t, unsigned long, size_t))
{
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
	size_t i;
	unsigned long increment;
	int rc;

	// increment = PAGE_SIZE;

	// for (i = 0; i < pages; i++) {
	// 	unsigned long current_paddr;

	// 	current_paddr = (paddr + i * increment);

	// 	rc = _page_map(pt, vaddr + i * increment, current_paddr, prot,
	// 		flags, is_initmem, pte_write);
	// 	// if (rc) {
	// 	// 	size_t j;

	// 	// 	uk_pr_err("Could not map page 0x%08lx\n",
	// 	// 		vaddr + i * increment);

	// 	// 	for (j = 0; j < i; j++)
	// 	// 		_page_unmap(pt, vaddr, pte_write);
	// 	// 	return -1;
	// 	// }
	// }

	unsigned long *pt_start = get_pt(vaddr);
	
	for (unsigned long i = 0; i < pages; i++) {
		//uk_pr_crit("Hiya\n");
		//unsigned long pte1 = pt_start[i];
		//pte1 = ukarch_pte_read(pt_start+i,L3_OFFSET(vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE)),3);
	 	unsigned long new_pte = L3_PT_ENTRY_TEMPLATE;
		new_pte |= paddr +(i*PAGE_SIZE);
#ifdef CONFIG_LIBSASSYFORK_SMP
		new_pte |= prot_to_pte(prot) | PTE_ATTR_SH(PTE_ATTR_SH_IS);
#else
		new_pte |= prot_to_pte(prot) | PTE_ATTR_SH(PTE_ATTR_SH_OS);
#endif
		//uk_pr_crit("vaddr %p\n", vaddr+(i*PAGE_SIZE));
	// 	unsigned long current_pte = new_pte | paddr;
		pt_start[i] = new_pte;
		
		//ukarch_tlb_flush();
		//uk_pr_crit("Vaddr %p\n", vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE));

	}
	//ukarch_tlb_flush_entry(vaddr+(i*PAGE_SIZE));
	ukarch_tlb_flush();

// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif

	return 0;
}

int uk_map_region(unsigned long vaddr, unsigned long paddr,
	unsigned long pages, unsigned long prot, unsigned long flags)
{
	//uk_pr_crit("dest %p, source %p, no of pages %x\n", vaddr, paddr, pages);
	return _map_region(ukarch_read_pt_base(), vaddr, paddr, pages,
		prot, flags, 0, ukarch_pte_write);
}

int uk_copy_ptes(unsigned long src_start_vaddr, unsigned long src_end_vaddr, unsigned long dst_start_vaddr)
{
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
	// get pte ptr
	unsigned long *pt_start = get_pt(src_start_vaddr);
	unsigned long *pt_end = get_pt(src_end_vaddr);
	unsigned long *pt_dst_start = get_pt(dst_start_vaddr);
	memcpy(pt_dst_start, pt_start, (unsigned long)pt_end-(unsigned long)pt_start);
	//ukarch_tlb_flush();
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif
	return 0;

}

int _initmem_map_region(unsigned long pt, unsigned long vaddr,
	unsigned long paddr, unsigned long pages, unsigned long prot,
	unsigned long flags)
{
	return _map_region(pt, vaddr, paddr, pages, prot, flags, 1,
		ukarch_pte_write);
}

int _page_set_prot(unsigned long* pt, unsigned long vaddr,
	unsigned long new_prot,
	int (*pte_write)(unsigned long*, size_t, unsigned long, size_t))
{
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
	unsigned long pte, new_pte;
	int rc;

	if (!PAGE_ALIGNED(vaddr)) {
	 	uk_pr_crit("Address must be aligned to page size\n");
	 	return -1;
	}

	pte = get_pte(vaddr);
	unsigned long paddr = (unsigned long)pt_pte_to_cap(pte, 3);
	pte = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);
	//uk_pr_crit("Got first level pte\n");
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 1);
	UK_ASSERT(__builtin_cheri_tag_get(pt) ==1);
	pte = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
	//uk_pr_crit("Got second level pte\n");
	// if (pte == PAGE_INVALID)
	//  		return -1;
	pt = pt_pte_to_cap(pte, 2);

	new_pte = L3_PT_ENTRY_TEMPLATE;
	new_pte |= paddr;
#ifdef CONFIG_LIBSASSYFORK_SMP
	new_pte |= prot_to_pte(new_prot) | PTE_ATTR_SH(PTE_ATTR_SH_IS);
#else
	new_pte |= prot_to_pte(new_prot) | PTE_ATTR_SH(PTE_ATTR_SH_OS);
#endif


	ukarch_pte_write(pt, L3_OFFSET(vaddr), new_pte, 3);
	ukarch_tlb_flush_entry(vaddr);
	//ukarch_tlb_flush();
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif
	return 0;
}

int uk_pages_set_prot(unsigned long vaddr_start, unsigned long pages,
	unsigned long new_prot)
{
#ifdef CONFIG_LIBSASSYFORK_SMP
	ukarch_spin_lock(&lock2);
#endif
	unsigned long* pt = ukarch_read_pt_base();
	unsigned long pte, new_pte;
	int rc;

	if (!PAGE_ALIGNED(vaddr_start)) {
	 	uk_pr_crit("Address must be aligned to page size\n");
	 	return -1;
	}

	unsigned long *pt_start = get_pt(vaddr_start);
	



	
	
	// pte = ukarch_pte_read(pt, L1_OFFSET(vaddr_start), 1);
	// unsigned long* pt_start = pt_pte_to_cap(pte, 1);
	// pte = ukarch_pte_read(pt_start, L2_OFFSET(vaddr_start), 2);
	// pt_start = pt_pte_to_cap(pte, 2);
	//
	for (unsigned long i = 0; i < pages; i++) {
		//uk_pr_crit("Hiya\n");
		unsigned long pte1 = pt_start[i];
		//pte1 = ukarch_pte_read(pt_start+i,L3_OFFSET(vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE)),3);
		unsigned long paddr = (unsigned long)pt_pte_to_cap(pte1, 3);
	 	new_pte = L3_PT_ENTRY_TEMPLATE;
		new_pte |= paddr;
#ifdef CONFIG_LIBSASSYFORK_SMP
		new_pte |= prot_to_pte(new_prot) | PTE_ATTR_SH(PTE_ATTR_SH_IS);
#else
		new_pte |= prot_to_pte(new_prot) | PTE_ATTR_SH(PTE_ATTR_SH_OS);
#endif
		//uk_pr_crit("New pte %lx\n", new_pte);
	// 	unsigned long current_pte = new_pte | paddr;
		pt_start[i] = new_pte;
		//ukarch_tlb_flush_entry(vaddr_start+(i*PAGE_SIZE));
		//ukarch_tlb_flush();
		//uk_pr_crit("Vaddr %p\n", vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE));

	}
	ukarch_tlb_flush();

#ifdef CONFIG_LIBSASSYFORK_SMP
	ukarch_spin_unlock(&lock2);
#endif

	return 0;
}

int uk_pages_set_prot_nocopa(unsigned long vaddr_start, unsigned long pages,
	unsigned long new_prot)
{
// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_lock(&lock2);
// #endif
	unsigned long* pt = ukarch_read_pt_base();
	unsigned long pte, new_pte;
	int rc;

	if (!PAGE_ALIGNED(vaddr_start)) {
	 	uk_pr_crit("Address must be aligned to page size\n");
	 	return -1;
	}

	unsigned long *pt_start = get_pt(vaddr_start);
	



	
	
	// pte = ukarch_pte_read(pt, L1_OFFSET(vaddr_start), 1);
	// unsigned long* pt_start = pt_pte_to_cap(pte, 1);
	// pte = ukarch_pte_read(pt_start, L2_OFFSET(vaddr_start), 2);
	// pt_start = pt_pte_to_cap(pte, 2);
	//
	for (unsigned long i = 0; i < pages; i++) {
		//uk_pr_crit("Hiya\n");
		unsigned long pte1 = pt_start[i];
		//pte1 = ukarch_pte_read(pt_start+i,L3_OFFSET(vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE)),3);
		unsigned long paddr = (unsigned long)pt_pte_to_cap(pte1, 3);
	 	new_pte = L3_PT_ENTRY_TEMPLATE;
		new_pte |= paddr;
#ifdef CONFIG_LIBSASSYFORK_SMP
		new_pte |= 0x2000000000000480 | PTE_ATTR_SH(PTE_ATTR_SH_IS);
#else
		new_pte |= 0x2000000000000480 | PTE_ATTR_SH(PTE_ATTR_SH_OS);
#endif
		//uk_pr_crit("New pte %lx\n", new_pte);
	// 	unsigned long current_pte = new_pte | paddr;
		pt_start[i] = new_pte;
		//ukarch_tlb_flush_entry(vaddr_start+(i*PAGE_SIZE));
		//ukarch_tlb_flush();
		//uk_pr_crit("Vaddr %p\n", vaddr_start+ADVANCE_POINTER(i*PAGE_SIZE));

	}
	ukarch_tlb_flush();

// #ifdef CONFIG_LIBSASSYFORK_SMP
// 	ukarch_spin_unlock(&lock2);
// #endif

	return 0;
}

int uk_page_set_prot(unsigned long vaddr, unsigned long new_prot)
{
	return _page_set_prot(ukarch_read_pt_base(), vaddr, new_prot,
		ukarch_pte_write);
}

int _initmem_page_set_prot(unsigned long pt, unsigned long vaddr,
	unsigned long new_prot)
{
	return _page_set_prot(pt, vaddr, new_prot, ukarch_pte_write);
}

// unsigned long _virt_to_pte(unsigned long pt, unsigned long vaddr)
// {
// 	unsigned long pt_entry;

// 	if (!PAGE_ALIGNED(vaddr)) {
// 		uk_pr_err("Address must be aligned to page size\n");
// 		return PAGE_NOT_MAPPED;
// 	}

// 	pt_entry = ukarch_pte_read(pt, L4_OFFSET(vaddr), 4);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return PAGE_NOT_MAPPED;

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);
// 	pt_entry = ukarch_pte_read(pt, L3_OFFSET(vaddr), 3);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return PAGE_NOT_MAPPED;
// 	if (PAGE_HUGE(pt_entry))
// 		return pt_entry;

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);
// 	pt_entry = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return PAGE_NOT_MAPPED;
// 	if (PAGE_LARGE(pt_entry))
// 		return pt_entry;

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);
// 	pt_entry = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);

// 	return pt_entry;
// }

// unsigned long uk_virt_to_pte(unsigned long vaddr)
// {
// 	return _virt_to_pte(ukarch_read_pt_base(), vaddr);
// }

// static unsigned long _initmem_pt_get(unsigned long paddr_start)
// {
// 	unsigned long frame = paddr_start + (_used_pts_count++) * PAGE_SIZE
// 		+ PAGETABLES_AREA_START - BOOKKEEP_AREA_START;

// 	// TODO
// 	// dupa ce booteaza Xen se termina memoria la adresa X
// 	// dupa X am garantati doar 512K
// 	// initial sunt mapati align_up(X, 4MB)
// 	// eu fac X + 2MB aici cand dau aceste frame-uri si e f posibil sa nu fie loc
// 	// uk_pr_err("frame is 0x%08lx\n", frame);

// 	memset((void *) frame, 0, PAGE_SIZE);

// 	return frame;
// }

// /**
//  * Create page tables that have mappings for themselves. Any other mappings
//  * can be then created using the API, after the value returned by this function
//  * is set as the PT base.
//  * @return PT base, the physical address of the 4th level page table.
//  */
// static unsigned long _pt_create(unsigned long paddr_start)
// {
// 	unsigned long pt_l4, pt_l3, pt_l2, pt_l1;
// 	unsigned long prev_l4_offset, prev_l3_offset, prev_l2_offset;
// 	unsigned long page, frame;

// 	pt_l4 = _initmem_pt_get(paddr_start);
// 	pt_l3 = _initmem_pt_get(paddr_start);
// 	pt_l2 = _initmem_pt_get(paddr_start);
// 	pt_l1 = _initmem_pt_get(paddr_start);

// 	prev_l4_offset = L4_OFFSET(BOOKKEEP_AREA_START);
// 	prev_l3_offset = L3_OFFSET(BOOKKEEP_AREA_START);
// 	prev_l2_offset = L2_OFFSET(BOOKKEEP_AREA_START);

// 	_ukarch_pte_write_raw(pt_l4, prev_l4_offset,
// 			(pfn_to_mfn(pt_l3 >> PAGE_SHIFT) << PAGE_SHIFT)
// 			| L4_PROT, 4);
// 	_ukarch_pte_write_raw(pt_l3, prev_l3_offset,
// 			(pfn_to_mfn(pt_l2 >> PAGE_SHIFT) << PAGE_SHIFT)
// 			| L3_PROT, 3);
// 	_ukarch_pte_write_raw(pt_l2, prev_l2_offset,
// 			(pfn_to_mfn(pt_l1 >> PAGE_SHIFT) << PAGE_SHIFT)
// 			| L2_PROT, 2);

// 	for (page = BOOKKEEP_AREA_START;
// 		page < BOOKKEEP_AREA_START + BOOKKEEP_AREA_SIZE;
// 		page += PAGE_SIZE) {
// 		if (L4_OFFSET(page) != prev_l4_offset) {
// 			pt_l3 = _initmem_pt_get(paddr_start);
// 			_ukarch_pte_write_raw(pt_l4, L4_OFFSET(page),
// 				(pfn_to_mfn(pt_l3 >> PAGE_SHIFT) << PAGE_SHIFT)
// 				| L4_PROT, 4);
// 			prev_l4_offset = L4_OFFSET(page);
// 		}

// 		if (L3_OFFSET(page) != prev_l3_offset) {
// 			pt_l2 = _initmem_pt_get(paddr_start);
// 			_ukarch_pte_write_raw(pt_l3, L3_OFFSET(page),
// 				(pfn_to_mfn(pt_l2 >> PAGE_SHIFT) << PAGE_SHIFT)
// 				| L3_PROT, 3);
// 			prev_l3_offset = L3_OFFSET(page);
// 		}

// 		if (L2_OFFSET(page) != prev_l2_offset) {
// 			pt_l1 = _initmem_pt_get(paddr_start);
// 			_ukarch_pte_write_raw(pt_l2, L2_OFFSET(page),
// 				(pfn_to_mfn(pt_l1 >> PAGE_SHIFT) << PAGE_SHIFT)
// 				| L2_PROT, 2);
// 			prev_l2_offset = L2_OFFSET(page);
// 		}

// 		frame = pfn_to_mfn((page - BOOKKEEP_AREA_START + paddr_start) >> PAGE_SHIFT) << PAGE_SHIFT;
// 		_ukarch_pte_write_raw(pt_l1, L1_OFFSET(page),
// 			frame | L1_PROT, 1);
// 	}

// 	return pt_l4;
// }

// void uk_pt_init(unsigned long pt_start_paddr, unsigned long paddr_start,
// 		size_t len)
// {
// 	unsigned long offset;

// 	unsigned long phys_bitmap_start_addr;
// 	size_t phys_bitmap_length;

// 	unsigned long phys_mem_start_addr;
// 	size_t phys_mem_length;

// 	if (!PAGE_ALIGNED(pt_start_paddr) ||
// 			(paddr_start != PAGE_PADDR_ANY
// 			 && !PAGE_ALIGNED(paddr_start))) {
// 		uk_pr_err("Addreses must be aligned to page size\n");
// 		return;
// 	}

// 	/*
// 	 * The needed bookkeeping internal structures are:
// 	 * - a physical address bitmap, to keep track of all available physical
// 	 *	 addresses (which will have a bit for every frame, so the size
// 	 *	 phys_mem_length / PAGE_SIZE)
// 	 * - a memory area where page tables are stored
// 	 * - a bitmap for pages used as page tables
// 	 */
// 	phys_mem_length = len;
// 	if (paddr_start == PAGE_PADDR_ANY)
// 		phys_mem_length -= BOOKKEEP_AREA_SIZE;

// 	/*
// 	 * If no specific area is given to be managed, the remaining memory is
// 	 * considered the actual usable memory.
// 	 */
// 	if (paddr_start == PAGE_PADDR_ANY) {
// 		phys_mem_start_addr =
// 			PAGE_ALIGN_UP(pt_start_paddr + PAGETABLES_AREA_START
// 					- BOOKKEEP_AREA_START
// 					+ PAGETABLES_AREA_SIZE);
// 	} else {
// 		phys_mem_start_addr = paddr_start;
// 	}

// 	offset = phys_mem_start_addr
// 		- PAGE_LARGE_ALIGN_DOWN(phys_mem_start_addr);
// 	phys_mem_start_addr -= offset;

// 	/*
// 	 * Need to bookkeep |phys_mem_length| bytes of physical
// 	 * memory, starting from |phys_mem_start_addr|. This is the
// 	 * physical memory given by the hypervisor.
// 	 *
// 	 * In Xen's case, the bitmap keeps the pseudo-physical addresses, the
// 	 * translation to machine frames being done later.
// 	 */

// 	pt_mem_start_addr = pt_start_paddr + PAGETABLES_AREA_START
// 		- BOOKKEEP_AREA_START;
// 	pt_mem_length = PAGETABLES_AREA_SIZE;


// 	/* Bookkeeping free pages used for PT allocations */
// 	pt_bitmap_start_addr = pt_start_paddr;
// 	pt_bitmap_length = pt_mem_length >> PAGE_SHIFT;
// 	uk_bitmap_zero((unsigned long *) pt_bitmap_start_addr,
// 			pt_bitmap_length);
// 	uk_bitmap_set((unsigned long *) pt_bitmap_start_addr, 0,
// 			_used_pts_count);

// 	phys_bitmap_start_addr = PAGE_ALIGN_UP(pt_bitmap_start_addr + pt_bitmap_length);
// 	phys_bitmap_length = (phys_mem_length + offset) >> PAGE_SHIFT;

// 	_phys_mem_region_list[_phys_mem_region_list_size].start_addr =
// 		phys_mem_start_addr;
// 	_phys_mem_region_list[_phys_mem_region_list_size].length = phys_mem_length;
// 	_phys_mem_region_list[_phys_mem_region_list_size].bitmap_start_addr =
// 		phys_bitmap_start_addr;
// 	_phys_mem_region_list[_phys_mem_region_list_size].bitmap_length =
// 		phys_bitmap_length;

// 	_phys_mem_region_list_size++;
// }

// int uk_pt_add_mem(unsigned long paddr_start, unsigned long len)
// {
// 	unsigned long bitmap_start_paddr;
// 	unsigned long bitmap_start_vaddr;
// 	size_t bitmap_length;

// 	unsigned long mem_start_addr;
// 	size_t mem_length;

// 	if (!_phys_mem_region_list_size) {
// 		uk_pr_err("When initializing the first chunk of physical memory, use uk_pt_init\n");
// 		return -1;
// 	}

// 	paddr_start = PAGE_ALIGN_UP(paddr_start);
// 	len = PAGE_ALIGN_DOWN(len);

// 	/*
// 	 * mem_length + bitmap_length = len
// 	 */
// 	mem_length = len * PAGE_SIZE / (PAGE_SIZE + 1);
// 	bitmap_start_paddr = paddr_start;
// 	bitmap_length = PAGE_ALIGN_UP(mem_length) >> PAGE_SHIFT;
// 	mem_start_addr = PAGE_ALIGN_UP(bitmap_start_paddr + bitmap_length);

// 	bitmap_start_vaddr = PAGE_ALIGN_UP(_phys_mem_region_list[_phys_mem_region_list_size - 1].bitmap_start_addr
// 		+ _phys_mem_region_list[_phys_mem_region_list_size - 1].bitmap_length);
// 	if (bitmap_start_vaddr + bitmap_length > PAGETABLES_AREA_START) {
// 		uk_pr_err("Not enough bookkeeping space\n");
// 		return -1;
// 	}

// 	uk_pr_err("bitmap start vaddr is 0x%08lx\n", bitmap_start_vaddr);
// 	uk_map_region(bitmap_start_vaddr, bitmap_start_paddr, bitmap_length >> PAGE_SHIFT, PAGE_PROT_READ | PAGE_PROT_WRITE, 0);

// 	uk_bitmap_zero((unsigned long *) bitmap_start_vaddr, bitmap_length);

// 	_phys_mem_region_list[_phys_mem_region_list_size].start_addr =
// 		mem_start_addr;
// 	_phys_mem_region_list[_phys_mem_region_list_size].length = PAGE_ALIGN_DOWN(mem_length);
// 	_phys_mem_region_list[_phys_mem_region_list_size].bitmap_start_addr =
// 		bitmap_start_vaddr;
// 	_phys_mem_region_list[_phys_mem_region_list_size].bitmap_length =
// 		bitmap_length;

// 	_phys_mem_region_list_size++;

// 	return 0;
// }

// #ifdef CONFIG_PLAT_KVM
// static int _mmap_kvm_areas(unsigned long pt_base)
// {
// 	unsigned long mbinfo_pages, vgabuffer_pages;

// 	mbinfo_pages = DIV_ROUND_UP(MBINFO_AREA_SIZE, PAGE_SIZE);
// 	vgabuffer_pages = DIV_ROUND_UP(VGABUFFER_AREA_SIZE, PAGE_SIZE);
// 	if (_initmem_map_region(pt_base, MBINFO_AREA_START, MBINFO_AREA_START,
// 			mbinfo_pages, PAGE_PROT_READ, 0))
// 		return -1;

// 	if (_initmem_map_region(pt_base, VGABUFFER_AREA_START,
// 			VGABUFFER_AREA_START, vgabuffer_pages,
// 			PAGE_PROT_READ | PAGE_PROT_WRITE, 0))
// 		return -1;

// 	return 0;
// }
// #endif /* CONFIG_PLAT_KVM */

// static int _mmap_kernel(unsigned long pt_base,
// 		unsigned long kernel_start_vaddr,
// 		unsigned long kernel_start_paddr,
// 		unsigned long kernel_area_size)
// {
// 	unsigned long kernel_pages;

// 	UK_ASSERT(PAGE_ALIGNED(kernel_start_vaddr));
// 	UK_ASSERT(PAGE_ALIGNED(kernel_start_paddr));

// #ifdef CONFIG_PLAT_KVM
// 	if (_mmap_kvm_areas(pt_base))
// 		return -1;
// #endif /* CONFIG_PLAT_KVM */

// 	/* TODO: break down into RW regions and RX regions */
// 	kernel_pages = DIV_ROUND_UP(kernel_area_size, PAGE_SIZE);
// 	if (_initmem_map_region(pt_base, kernel_start_vaddr,
// 			kernel_start_paddr, kernel_pages,
// 			PAGE_PROT_READ | PAGE_PROT_WRITE | PAGE_PROT_EXEC, 0))
// 		return -1;

// 	/*
// 	 * It is safe to return from this function, since we are still on the
// 	 * bootstrap stack, which is in the bss section, in the binary.
// 	 * The switch to another stack is done later.
// 	 */
// 	return 0;
// }

// static int _initmem_set_prot_region(unsigned long pt_base, unsigned long vaddr,
// 	unsigned long len, unsigned long new_prot)
// {
// 	unsigned long page;
// 	int rc;

// 	for (page = vaddr; page < vaddr + len; page += PAGE_SIZE) {
// 		rc = _initmem_page_set_prot(pt_base, page, new_prot);
// 		if (rc)
// 			return -1;
// 	}

// 	return 0;
// }

// static int uk_set_prot_region(unsigned long vaddr, unsigned long len,
// 	unsigned long new_prot)
// {
// 	unsigned long page;
// 	int rc;

// 	for (page = vaddr; page < vaddr + len; page += PAGE_SIZE) {
// 		rc = uk_page_set_prot(page, new_prot);
// 		if (rc)
// 			return -1;
// 	}

// 	return 0;
// }

// void uk_pt_build(unsigned long paddr_start, unsigned long len,
// 	unsigned long kernel_start_vaddr,
// 	unsigned long kernel_start_paddr,
// 	unsigned long kernel_area_size)
// {
// 	unsigned long pt_base;

// 	UK_ASSERT(PAGE_ALIGNED(paddr_start));
// 	UK_ASSERT(PAGE_ALIGNED(len));

// 	uk_pr_err("paddr_start is 0x%08lx\n", paddr_start);
// 	pt_base = _pt_create(paddr_start);
// 	uk_pt_init(paddr_start, PAGE_PADDR_ANY, len);
// 	if (_mmap_kernel(pt_base, kernel_start_vaddr, kernel_start_paddr,
// 				kernel_area_size))
// 		UK_CRASH("Could not map kernel\n");

// #ifdef CONFIG_PARAVIRT
// 	_initmem_page_map(pt_base, SHAREDINFO_PAGE,
// 			PTE_REMOVE_FLAGS(uk_virt_to_pte(SHAREDINFO_PAGE)),
// 			PAGE_PROT_READ | PAGE_PROT_WRITE, 0);
// 	/* All pagetables must be set to read only before writing new pt_base */
// 	_initmem_set_prot_region(pt_base, PAGETABLES_AREA_START,
// 			PAGETABLES_AREA_SIZE, PAGE_PROT_READ);
// 	uk_set_prot_region(pt_base, PAGETABLES_AREA_SIZE, PAGE_PROT_READ);
// #endif /* CONFIG_PARAVIRT */

// 	ukarch_write_pt_base(pt_base);
// 	_virt_offset = PAGETABLES_AREA_START - pt_base;
// 	_phys_mem_region_list[0].bitmap_start_addr += _virt_offset;
// 	pt_bitmap_start_addr += _virt_offset;

// 	uk_bitmap_zero((unsigned long *) _phys_mem_region_list[0].bitmap_start_addr,
// 			_phys_mem_region_list[0].bitmap_length);
// 	uk_bitmap_set((unsigned long *) _phys_mem_region_list[0].bitmap_start_addr, 0,
// 			(_phys_mem_region_list[0].start_addr - PAGE_LARGE_ALIGN_DOWN(_phys_mem_region_list[0].start_addr))>> PAGE_SHIFT);
// 	uk_bitmap_zero((unsigned long *) stack_bitmap_start_addr,
// 			stack_bitmap_length);
// }

// void *uk_stack_alloc()
// {
// 	unsigned long stack_start_vaddr;
// 	unsigned long offset;

// 	offset = uk_bitmap_find_next_zero_area(
// 			(unsigned long *) stack_bitmap_start_addr,
// 			stack_bitmap_length,
// 			0 /* start */, 1 /* nr */, 0 /* align_mask */);

// 	if (offset > STACK_COUNT) {
// 		uk_pr_err("No more stacks available\n");
// 		return NULL;
// 	}

// 	uk_bitmap_set((unsigned long *) stack_bitmap_start_addr, offset, 1);

// 	/* Map stack in regular pages */
// 	stack_start_vaddr = STACK_AREA_START + offset * __STACK_SIZE;
// 	if (uk_map_region(stack_start_vaddr, PAGE_PADDR_ANY,
// 				__STACK_SIZE >> PAGE_SHIFT,
// 				PAGE_PROT_READ | PAGE_PROT_WRITE, 0))
// 		return NULL;

// 	return (void *) stack_start_vaddr;
// }

// int uk_stack_free(void *vaddr)
// {
// 	unsigned long pages;
// 	size_t i;

// 	if ((unsigned long) vaddr < STACK_AREA_START
// 		|| (unsigned long) vaddr > (STACK_AREA_END - __STACK_SIZE)
// 		|| (((unsigned long) vaddr) & (__STACK_SIZE - 1)))
// 		return -1;

// 	pages = __STACK_SIZE >> PAGE_SHIFT;
// 	for (i = 0; i < pages; i++) {
// 		if (uk_page_unmap(((unsigned long) vaddr) + i * PAGE_SIZE)) {
// 			uk_pr_err("Page 0x%08lx not previously mapped\n",
// 				((unsigned long) vaddr) + i * PAGE_SIZE);

// 			return -1;
// 		}
// 	}

// 	return 0;
// }

// int uk_heap_map(unsigned long vaddr, unsigned long len)
// {
// 	unsigned long heap_pages, heap_large_pages;

// 	if (vaddr < HEAP_AREA_START || vaddr + len > HEAP_AREA_END)
// 		return -1;

// /* note: we do not want to use large pages for the heap in the case of Intel
//  * PKU; large pages prevent us from being able to set page protections at a
//  * 4K granularity and result in unintuitive and undesired behavior. */
// #if (CONFIG_PARAVIRT || CONFIG_LIBFLEXOS_INTELPKU)
// 	if (uk_map_region(vaddr, PAGE_PADDR_ANY,
// 			len >> PAGE_SHIFT, PAGE_PROT_READ | PAGE_PROT_WRITE, 0))
// 		return -1;
// #else /* CONFIG_PARAVIRT */
// 	/* Map heap in large and regular pages */
// 	heap_large_pages = len >> PAGE_LARGE_SHIFT;

// 	if (uk_map_region(vaddr, PAGE_PADDR_ANY,
// 			heap_large_pages,
// 			PAGE_PROT_READ | PAGE_PROT_WRITE,
// 			PAGE_FLAG_LARGE))
// 		return -1;

// 	/*
// 	 * If the heap is not properly aligned to PAGE_LARGE_SIZE,
// 	 * map the rest in regular pages
// 	 */
// 	if ((heap_large_pages << PAGE_LARGE_SHIFT) < len) {
// 		heap_pages = (len - (heap_large_pages << PAGE_LARGE_SHIFT))
// 			>> PAGE_SHIFT;
// 	} else {
// 		heap_pages = 0;
// 	}

// 	if (uk_map_region(vaddr + (heap_large_pages << PAGE_LARGE_SHIFT),
// 				PAGE_PADDR_ANY, heap_pages,
// 				PAGE_PROT_READ | PAGE_PROT_WRITE, 0))
// 		return -1;
// #endif /* CONFIG_PARAVIRT */

// 	return 0;
// }

// void dump_pt(unsigned long pt, unsigned long vaddr)
// {
// 	unsigned long pt_entry;
// 	size_t i;

// 	if (!PAGE_ALIGNED(vaddr)) {
// 		uk_pr_err("Address must be aligned to page size\n");
// 		return;
// 	}

// 	printf("L4 table for address 0x%08lx is 0x%08lx\n", vaddr, pt);
// 	for (i = 0; i < L4_OFFSET(vaddr) || i < 2; i += 2)
// 		printf("0x%08lx: 0x%08lx 0x%08lx\n", pt + 8 * i,
// 				*((unsigned long *) pt + i),
// 				*((unsigned long *) pt + i + 1));

// 	pt_entry = ukarch_pte_read(pt, L4_OFFSET(vaddr), 4);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return;

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);

// 	printf("L3 table for address 0x%08lx is 0x%08lx\n", vaddr, pt);
// 	for (i = 0; i < L3_OFFSET(vaddr) || i < 2; i += 2)
// 		printf("0x%08lx: 0x%08lx 0x%08lx\n", pt + 8 * i,
// 				*((unsigned long *) pt + i),
// 				*((unsigned long *) pt + i + 1));

// 	pt_entry = ukarch_pte_read(pt, L3_OFFSET(vaddr), 3);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return;
// 	if (PAGE_HUGE(pt_entry)) {
// 		printf("PTE for vaddr 0x%08lx is 0x%08lx\n", vaddr, pt_entry);
// 		return;
// 	}

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);

// 	printf("L2 table for address 0x%08lx is 0x%08lx\n", vaddr, pt);
// 	for (i = 0; i < L2_OFFSET(vaddr) || i < 2; i += 2)
// 		printf("0x%08lx: 0x%08lx 0x%08lx\n", pt + 8 * i,
// 				*((unsigned long *) pt + i),
// 				*((unsigned long *) pt + i + 1));

// 	pt_entry = ukarch_pte_read(pt, L2_OFFSET(vaddr), 2);
// 	if (!PAGE_PRESENT(pt_entry))
// 		return;
// 	if (PAGE_LARGE(pt_entry)) {
// 		printf("Large page PTE for vaddr 0x%08lx is 0x%08lx\n",
// 				vaddr, pt_entry);
// 		return;
// 	}

// 	pt = (unsigned long) pt_pte_to_virt(pt_entry);

// 	printf("L1 table for address 0x%08lx is 0x%08lx\n", vaddr, pt);
// 	for (i = 0; i < L1_OFFSET(vaddr) || i < 2; i += 2)
// 		printf("0x%08lx: 0x%08lx 0x%08lx\n", pt + 8 * i,
// 				*((unsigned long *) pt + i),
// 				*((unsigned long *) pt + i + 1));

// 	pt_entry = ukarch_pte_read(pt, L1_OFFSET(vaddr), 1);

// 	printf("PTE for vaddr 0x%08lx is 0x%08lx\n", vaddr, pt_entry);
// }


