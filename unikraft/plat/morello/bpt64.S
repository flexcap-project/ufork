/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Michalis Pappas <michalis.pappas@opensynergy.com>
 *
 * Based on plat/kvm/x86/pagetable64.S by Marc Rittinghaus.
 *
 * Copyright (c) 2022, OpenSynergy GmbH. All rights reserved.
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
#include <uk/config.h>
#include <uk/arch/lcpu.h>
//#include <uk/arch/paging.h>
#include <uk/asm.h>
#include <arm/arm64/cpu_defs.h>

#define PTE_VALID_BIT			1
#define PTE_TYPE_MASK			0x3
#define PTE_TYPE_BLOCK			1
#define PTE_TYPE_PAGE			3
#define PTE_TYPE_TABLE			3

/* Translation Table Descriptor Attributes */
#define PTE_ATTR_MASK_H			_AC(0xfff0000000000000, UL)
#define PTE_ATTR_MASK_L			_AC(0x0000000000000fff, UL)
#define PTE_ATTR_MASK			(ATTR_MASK_H | ATTR_MASK_L)
#define PTE_ATTR_SW_MANAGED		(_AC(1, UL) << 56)
#define PTE_ATTR_SW_WIRED		(_AC(1, UL) << 55)
#define PTE_ATTR_UXN			(_AC(1, UL) << 54)
#define PTE_ATTR_PXN			(_AC(1, UL) << 53)
#define PTE_ATTR_XN			(PTE_ATTR_PXN | PTE_ATTR_UXN)
#define PTE_ATTR_CONTIGUOUS		(_AC(1, UL) << 52)
#define PTE_ATTR_DBM			(_AC(1, UL) << 51)
#define PTE_ATTR_GP			(_AC(1, UL) << 50)
#define PTE_ATTR_nG			(1 << 11)
#define PTE_ATTR_AF			(1 << 10)
#define PTE_ATTR_SH(x)			((x) << 8)
#define PTE_ATTR_SH_MASK		PTE_ATTR_SH(3)
#define PTE_ATTR_SH_NS			0 /* Non-shareable */
#define PTE_ATTR_SH_OS			2 /* Outer-shareable */
#define PTE_ATTR_SH_IS			3 /* Inner-shareable */
#define PTE_ATTR_AP_RW_BIT		(1 << 7)
#define PTE_ATTR_AP(x)			((x) << 6)
#define PTE_ATTR_AP_MASK		ATTR_AP(3)
#define PTE_ATTR_AP_RW			(0 << 1)
#define PTE_ATTR_AP_RO			(1 << 1)
#define PTE_ATTR_AP_USER		(1 << 0)
#define PTE_ATTR_NS			(1 << 5)
#define PTE_ATTR_IDX(x)			((x) << 2)
#define PTE_ATTR_IDX_MASK		(7 << 2)

#define CACHE_LINE_SIZE		64

/* Device-nGnRnE memory */
#define MAIR_DEVICE_nGnRnE	0x00
/* Device-nGnRE memory */
#define MAIR_DEVICE_nGnRE	0x04
/* Device-GRE memory */
#define MAIR_DEVICE_GRE		0x0C
/* Outer Non-cacheable + Inner Non-cacheable */
#define MAIR_NORMAL_NC		0x44
/* Outer + Inner Write-back non-transient */
#define MAIR_NORMAL_WB		0xff
/* Tagged Outer + Inner Write-back non-transient */
#define	MAIR_NORMAL_WB_TAGGED	0xf0
/* Outer + Inner Write-through non-transient */
#define MAIR_NORMAL_WT		0xbb

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

#define PTE_BLOCK_DEVICE_nGnRnE	(PTE_ATTR_DEVICE_nGnRnE + PTE_TYPE_BLOCK)
#define PTE_BLOCK_DEVICE_nGnRE	(PTE_ATTR_DEVICE_nGnRE + PTE_TYPE_BLOCK)
#define PTE_BLOCK_NORMAL_RW	(PTE_ATTR_NORMAL_RW + PTE_TYPE_BLOCK)
#define PTE_BLOCK_NORMAL_RWX	(PTE_ATTR_NORMAL_RWX + PTE_TYPE_BLOCK)
#define PTE_PAGE_NORMAL_RO	(PTE_ATTR_NORMAL_RO  + PTE_TYPE_PAGE)
#define PTE_PAGE_NORMAL_RWX	(PTE_ATTR_NORMAL_RWX  + PTE_TYPE_PAGE)


#define PAGE_Lx_SHIFT(lvl)					\
	(PAGE_SHIFT + (PT_LEVEL_SHIFT * lvl))

/**
 * PAGE_Lx_SIZE(lvl)
 *
 * @param lvl a page table level [0..PT_LEVELS - 1]
 *
 * @return the size of a page at the given level. Must return the
 *    appropriate size even if the architecture does not support configuring
 *    a page at this level
 */
#ifndef PAGE_Lx_SIZE
#define PAGE_Lx_SIZE(lvl)		(1UL << PAGE_Lx_SHIFT(lvl))
#endif

/**
 * Outputs a single table descriptor
 *
 * @param paddr physical address of the linked PT
 */
.macro pte_tbl_desc paddr
	.quad \paddr + PTE_TYPE_TABLE
.endm

/**
 * Outputs a single block / page descriptor
 *
 * @param paddr physical address
 * @param attr  PT attributes
 */
.macro pte_map_desc paddr, attr
	.quad \paddr + \attr
.endm

/* Outputs a number of block / table descriptors for a contiguous
 * mapping, starting at the provided physical address.
 *
 * @param paddr physical address of the beginning of the area to map
 * @param pages number of pages to map
 * @param level level of the page table the PTEs are intended for
 * @param pte additional flags for PTE
 */
.macro pte_fill paddr, pages, lvl, attr
.ifle \pages
	.exitm
.endif
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 0), \attr
.ifgt (\pages - 8)
	/* Have to do some unrolling to not exceed max nested macros */
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 1), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 2), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 3), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 4), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 5), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 6), \attr
	pte_map_desc (\paddr + PAGE_Lx_SIZE(\lvl) * 7), \attr
	pte_fill    (\paddr + PAGE_Lx_SIZE(\lvl) * 8), (\pages - 8), \lvl, \attr
.else
	pte_fill (\paddr + PAGE_Lx_SIZE(\lvl) * 1), (\pages - 1), \lvl, \attr
.endif
.endm

/* Outputs a number of invalid PTEs
 *
 * @param paddr physical address (not used)
 * @param pages number of PTEs
 */
.macro pte_zero paddr=0, pages
	.fill \pages, 0x8, 0
.endm

/* ----------------------- KVM MEMORY MAP (QEMU virt) ---------------------
 *
 * 0x0000000000000000 - 0x0000000007ffffff	Hole:          0    - 128MiB
 * 0x0000000008000000 - 0x000000003fffffff	Devices:     128MiB -   1GiB
 * 0x0000000040000000 - 0x0000007fffffffff	Kernel:        1GiB - 512GiB
 * 0x0000008000000000 - 0x000000ffffffffff	PCIe hi mem: 512GiB -   1TiB
 *
 * Notice: The page tables below use the Unikraft indexing convention (x86).
 */
.section .data

.global arm64_bpt_l3_pt0
.global arm64_bpt_l2_pt0

/* L3: 0 - 2TiB (512GiB / entry)
 *
 * 0x0000000000000000 - 0x0000007fffffffff	Table descriptor to l2_pt0
 * 0x0000008000000000 - 0x000000ffffffffff	Table descriptor to l2_pt1
 * 0x0000010000000000 - 0x0000ff7fffffffff	Unmapped
 * 0x0000ff8000000000 - 0x0000ffffffffffff	Table descriptor to l2_pt511
 */
.align 12
arm64_bpt_l3_pt0:
	pte_tbl_desc	arm64_bpt_l2_pt0
	pte_tbl_desc	arm64_bpt_l2_pt1
//	pte_zero	, 510

/* L2: 0 - 512GiB (1GiB / entry)
 *
 * 0x0000000000000000 - 0x000000003fffffff	Table descriptor to l1_pt0
 * 0x0000000040000000 - 0x000000007fffffff	Table descriptor to l1_pt1
 * 0x0000000080000000 - 0x000000003fffffff	RAM       @   2GiB
 * 0x0000000400000000 - 0x0000007fffffffff	PCIe ECAM @ 256GiB
 */
.align 12
arm64_bpt_l2_pt0:
	pte_tbl_desc	arm64_bpt_l1_pt0
//	pte_tbl_desc	arm64_bpt_l1_pt1
	pte_fill	0x0000000080000000, 254, 2, PTE_BLOCK_DEVICE_nGnRnE
	pte_fill	0x0000004000000000, 256, 2, PTE_BLOCK_DEVICE_nGnRnE

/* L2: 512GiB - 1TiB (1GiB / entry)
 *
 * 0x0000008000000000 - 0x000000ffffffffff	PCIe hi-mem @ 512GiB
 */
.align 12
arm64_bpt_l2_pt1:
	pte_fill	0x0000008000000000, 512, 2, PTE_BLOCK_DEVICE_nGnRE

/* L1: 0 - 1GiB (2MiB / entry)
 *
 * 0x0000000000000000 - 0x0000000007ffffff	Hole:   @ 0
 * 0x0000000008000000 - 0x000000003fffffff	Devices @ 128MiB
 */
.align 12
arm64_bpt_l1_pt0:
	pte_fill	0x0000000000000000, 64, 1, PTE_BLOCK_DEVICE_nGnRE
	pte_fill	0x0000000008000000, 448, 1, PTE_BLOCK_DEVICE_nGnRE

// /* L1: 1GiB - 2GiB (2MiB / entry)
//  *
//  * 0x0000000000000000 - 0x00000000001fffff	Table descriptor to l0_pt0
//  * 0x0000000040200000 - 0x000000007fffffff	RAM	@ 1.2GiB
//  */
// .align 12
// arm64_bpt_l1_pt1:
// 	pte_tbl_desc	arm64_bpt_l0_pt0
// 	pte_fill	0x0000000040200000, 511, 1, PTE_BLOCK_NORMAL_RWX

// /* L0: 1GiB - 1.2GiB (4KiB / entry)
//  *
//  * 0x0000000040000000 - 0x00000000401fffff	Kernel	@ 1GiB
//  */
// .align 12
// arm64_bpt_l0_pt0:
// 	pte_fill	0x0000000040000000, 256, 0, PTE_PAGE_NORMAL_RO
// 	pte_fill	0x0000000040100000, 256, 0, PTE_PAGE_NORMAL_RWX