/*
 * Copyright (C) 2018, bzt (bztsrc@github), https://github.com/bztsrc/raspi3-tutorial
 * Copyright (c) 2018, Sergey Matyukevich <https://github.com/s-matyukevich/raspberry-pi-os>
 *           (c) 2020, Santiago Pagani <santiagopagani@gmail.com>
 * Copyright (c) 2022, John A. Kressel <jkressel.apps@gmail.com>
 *
 *
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <morello/sysregs.h>
#include <morello/mm.h>
#include <morello/mmu.h>
#include <uk/config.h>
#include <arm/cpu_defs.h>
#include <uk/asm.h>
#include <uk/arch/limits.h>

#define DEVICE_ADDR_START 0
#define DEVICE_ADDR_SIZE  0x40000000
#define RAM_ADDR_START    0x40000000
#define RAM_L2_ENTRIES    255
#define RAM_ADDR_SIZE     (0x40000000 * RAM_L2_ENTRIES)
#define PCIE_ADDR_START   0x8000000000
#define PCIE_L2_ENTRIES   512
#define PCIE_ADDR_SIZE    0x8000000000

#define TT_S1_TABLE	0x3
#define TT_S1_INNER_SHARED    (3 << 8)
#define TT_S1_NORMAL_WBWA     0x00000000000000405
#define L2_ENTRY_SIZE         2 * (1 << 20)

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

.global __start
__start:
mov x26, x0

//hlt 0
//toself: b toself
setup_cpu:

	/*
	* First make sure that morello features don't get trapped
	* What controls access to morello I hear you ask? 
	* From the Morello architecture specification:
	* 1. cptr_el3, EC (bit 9): 0b0 causes traps, 0b1 no trap to el3
	* 2. cptr_el2, CEN (bits 18 and 19): bit pattern 0b11 causes no traps to el2
	* 3. cptr_el2, TC (bit 9): 0b0 causes no morello instructions to be trapped
	* 4. cpacr_el1, CEN (bits 18 and 19): 0b11 causes no instruction traps
	* Also while I'm at it, disable fp traps at el1.
	* 
	*/



//	ldr x0, =(3 << 18) //CEN (bits 18 and 19) are 1 and TC (bit 9) is 0
//	msr cptr_el2, x0

	ldr x1, =(3 << 20) //fp
	ldr x2, =(3 << 18) // CEN (bits 18 and 19) set to 1
	orr x0, x1, x0
	orr x0, x0, x2
	msr cpacr_el1, x0 //enable fp and capabilities

	isb


	// Set up VMPIDR_EL2/VPIDR_EL1
	// ---------------------------
//	mrs x0, midr_el1
//	msr vpidr_el2, x0
//	mrs x0, mpidr_el1
//	msr vmpidr_el2, x0

	// Set up exception vectors	
	// --------------------------------
	ldr x0, =vectors_el1
	cvtp c0, x0
	add c0, c0, #1
	msr cvbar_el1, c0
	
	// Set VMID
	// ---------
	// Although we are not using stage 2 translation, NS.EL1 still cares
	// about the VMID
//	msr vttbr_el2, xzr


	// Set SCTLRs for EL1/2 to safe values
	// ------------------------------------
//	msr sctlr_el2, xzr
	msr sctlr_el1, xzr



	// Configure HCR_EL2
	// ------------------
	orr w0, wzr, #(1 << 3)               // FMO=1
	orr x0, x0,  #(1 << 4)               // IMO=1
	orr x0, x0,  #(1 << 31)              // RW=1          NS.EL1 is AArch64
													// TGE=0         Entry to NS.EL1 is possible
													// VM=0          Stage 2 MMU disabled
//	msr hcr_el2, x0




	// Set the SPSR state to restore when returning from EL3 to EL1
	// ------------------------------------------------------------
//	ldr	x0, =(0x05)
//	msr	spsr_el2, x0

	ldr	x0, =(0x05)
	msr	spsr_el1, x0

	ldr x0, =(el1_entry)
	cvtp c0, x0
	gcperm x4, c0
	ldr x3, =(1 << 9)
	and x4, x4, x3

//	msr	celr_el2, c0

	mrs x0, cctlr_el1
	orr x0, x0, #(0x1 << 0)
	orr x0, x0, #(0x1 << 5)
	bic x0, x0, #(0x1 << 7) //don't want return caps to be sealed, you may want this behaviour enabled though

	msr cctlr_el1, x0


	ldr x0, =tt_l1_base //get the base page table
	msr ttbr0_el1, x0 

	// ldr x0, =_pagetables1 //get the base page table
	// msr ttbr1_el1, x0 


//	eret  //jump down to el1


//We are now at EL1
el1_entry:
    

	// Set up memory attributes
	// -------------------------
	// This equates to:
	// 0 = b01000100 = Normal, Inner/Outer Non-Cacheable
	// 1 = b11111111 = Normal, Inner/Outer WB/WA/RA
	// 2 = b00000000 = Device-nGnRnE
	mov x0, #0x000000000000FF44
	msr mair_el1, x0



	// Set up TCR_EL1
	// ---------------
	mov x0, #0x19             // T0SZ=0b011001 Limits VA space to 39 bits
	orr x0, x0, #(0x1 << 8)   // IGRN0=0b01  Walks to TTBR0 are Inner WB/WA
	orr x0, x0, #(0x1 << 10)  // OGRN0=0b01  Walks to TTBR0 are Outer WB/WA
	orr x0, x0, #(0x3 << 12)  // SH0=0b11   Inner Shareable
	orr x0, x0, #(0x1 << 23)  // EPD1=0b1   Disable table walks from TTBR1
	orr x0, x0, #(0x1 << 41)  // Don't fault capability loads and stores
	orr x0, x0, #(0x1 << 44)  // Don't fault capability stores
	orr x0, x0, #(0x1 << 45)  // Don't fault capability loads
	orr x0, x0, #(0x1 << 46)  // Don't fault capability loads
	orr x0, x0, #(0x1 << 60)  // Don't fault capability stores
	orr x0, x0, #(0x1 << 61)	//Don't fault capability loads
	orr x0, x0, #(0x1 << 62)

								// TBI0=0b0
								// TG0=0b00   4KB granule for TTBR0
								// A1=0     TTBR0 contains the ASID
								// AS=0     8-bit ASID
								// IPS=0     32-bit IPA space
	orr x0, x0, #(0x2 << 32)

	mov x9, x0
	msr tcr_el1, x0


	dsb   sy
	isb

	ldr   x1, =_pagetables1
	cvtp  c1, x1


	// [0]: 0x0000,0000 - 0x3FFF,FFFF
	ldr      x0, =0x00600000000000409          // Entry template
												// Don't need to OR in address, as it is 0
												// AP=0b00, EL1 RW, EL0 No Access
	str      x0, [c1]


	
	// [1]: 0x4000,0000 - 0x7FFF,FFFF
	ldr      x0, =0x00600000000000409          // Entry template
	orr      x0, x0, #0x40000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str      x0, [c1, #8]



	// [2]: 0x8000,0000 - 0xBFFF,FFFF (DRAM on the VE and Base Platform)
	ldr      x0, =0x3000000000000405           // Entry template
	orr      x0, x0, #0x80000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str      x0, [c1, #16]



	// [3]: 0xC0000000 - 0xFFFF,FFFF (DRAM on the VE and Base Platform)
	ldr      x0, =0x3000000000000405            // Entry template
	orr      x0, x0, #0xC0000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str      x0, [c1, #24]

	dsb      sy

		// [3]: 0xC0000000 - 0xFFFF,FFFF (DRAM on the VE and Base Platform)
	ldr      x0, =0x3000000000000405            // Entry template
	orr      x0, x0, #0x100000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str      x0, [c1, #32]

	dsb      sy


//	Set up blocks

	ldr x1, =tt_l1_base // base of l1 table
	cvtp c1, x1


	ldr x0, =0x00600000000000409          // Entry template
												// Don't need to OR in address, as it is 0
												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1]


	
	// [1]: 0x4000,0000 - 0x7FFF,FFFF
	ldr x0, =0x00600000000000409          // Entry template
	orr x0, x0, #0x40000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1, #8]



	// [2]: 0x8000,0000 - 0xBFFF,FFFF (DRAM on the VE and Base Platform)
	ldr x0, =0x3000000000000405           // Entry template
	orr x0, x0, #0x80000000               // 'OR' template with base physical address
												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1, #16]



	// [3]: 0xC0000000 - 0xFFFF,FFFF (DRAM on the VE and Base Platform)

	ldr x2, =0xC0000000           // Get address of L2 table
 	ldr x0, =0x3000000000000405          // Entry template for pointer to next level table
 	orr x0, x0, x2                // Combine template with L2 table Base address

												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1, #24]
	dsb sy


		// [3]: 0xC0000000 - 0xFFFF,FFFF (DRAM on the VE and Base Platform)

	ldr x2, =tt_l2_base           // Get address of L2 table
 	ldr x0, =TT_S1_TABLE          // Entry template for pointer to next level table
 	orr x0, x0, x2                // Combine template with L2 table Base address

												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1, #32]
	dsb sy







	ldr x2, =tt_l2_base           // Get address of L2 table
 	ldr x0, =TT_S1_TABLE          // Entry template for pointer to next level table
 	orr x0, x0, x2                // Combine template with L2 table Base address

												// AP=0b00, EL1 RW, EL0 No Access
	str x0, [c1, #32]
	dsb sy




	mov x2, #0 // counter for number of entries


	ldr x1, =tt_l2_base             // Address of L2 table
	cvtp c1, x1

create_level2:


	mov x3, #PAGE_SIZE
	mul x3, x3, x2				//Get the correct offset
	ldr x4, =_pagetables        // Get address of L3 table
	add x4, x4, x3
 	ldr x0, =TT_S1_TABLE        // Entry template for pointer to next level table
 	orr x0, x0, x4              // Combine template with L2 table Base address
	mov x6, #8
	mul x3, x2, x6
 	str x0, [c1, x3]
	add x2, x2, #1
	sub x3, x2, #512
	cbnz x3, create_level2

	dsb sy




// Now we are at the level three table

	mov x7, #0 //number of tables to build 512
	mov x4, #0x100000000 //starting address for l3

start_level_3:
	mov x8, #0 //entries per table 512

	ldr x1, =_pagetables             // Address of L3 table
	mov x2, #PAGE_SIZE
	mul x2, x2, x7
	add x1, x1, x2
	cvtp c1, x1

create_level3:

	ldr x0, =0x3000000000000707 // entry template
	
 	orr x0, x0, x4         // 'OR' template with base physical address
	orr x0, x0, #0x3
 	str x0, [c1]
	add c1, c1, #8
	add x4, x4, #PAGE_SIZE
	add x8, x8, #1
	sub x3, x8, #512
	cbnz x3, create_level3

	add x7, x7, #1
	sub x3, x7, #512
	cbnz x3, start_level_3

	dsb sy

	ldr x0, =(0x100000000)
	ldr x1, =(0x30000000)
	/* Get information about the caches from CTR_EL0 */
	mrs	x4, ctr_el0
	mov	x2, #4

	/* Get minimum D cache line size */
	ubfx	x3, x4, #CTR_DMINLINE_SHIFT, #CTR_DMINLINE_WIDTH
	lsl	x3, x2, x3

	/* Align the start address to line size */
	sub	x4, x3, #1
	and	x2, x0, x4
	add	x1, x1, x2
	bic	x0, x0, x4
1:
	/* clean and invalidate D cache by D cache line size */
	cvtp c0, x0
	dc	civac, c0
	dsb	nsh

	/* Move to next line and reduce the size */
	add	x0, x0, x3
	subs	x1, x1, x3

	/* Check if all range has been invalidated */
	b.hi	1b

	isb


/* Time to enable the mmu */

 	mov x0, #(1 << 0)                     // M=1           Enable the stage 1 MMU
  	orr x0, x0, #(1 << 2)                 // C=1           Enable data and unified caches
  	orr x0, x0, #(1 << 12) 
	orr x0, x0, #(1 << 29)               // I=1           Enable instruction fetches to allocate into unified caches
                                             // A=0           Strict alignment checking disabled
                                             // SA=0          Stack alignment checking disabled
                                             // WXN=0         Write permission does not imply XN
                                             // EE=0          EL3 data accesses are little endian
	msr SCTLR_EL1, x0
//	toself: b toself
	isb


	//
	// MMU is now enabled
	//


	nop
	nop
	nop
	nop

	//setup stack pointer to use
	msr SPSel, #0
	ldr x1, =__stack_end
	and x1, x1, #-16
	cvtp c1, x1
	mov csp, c1
	msr SPSel, #1
	mov csp, c1
	msr SPSel, #0



clear_bss_start:
	// Clear bss
	ldr x1, =__start_bss
	cvtp c1, x1
	ldr w2, =_bss_size
clear_bss_loop:
	cbz w2, clear_bss_done
	str xzr, [c1], #8
	sub w2, w2, #1
	cbnz w2, clear_bss_loop
clear_bss_done:

set_dtb:
	cvtp c26, x26
	mov c0, c26


jump_to_C:
	//load c entry point
	//before: b before
	ldr x4, =_libmorelloplat_entry
	cvtp c4, x4
	br c4


.weak   __cap_relocs_start
.weak   __cap_relocs_end

.align 4
.LCRStart:
  .dword __cap_relocs_start
  .globl __getCapRelocsStart
  .type  __getCapRelocsStart,@function
  .align 2
__getCapRelocsStart:
  adr c0, .LCRStart
  ldr x0, [c0]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret

  .align 4
.LCREnd:
  .dword __cap_relocs_end
  .globl __getCapRelocsEnd
  .type  __getCapRelocsEnd,@function
  .align 2
__getCapRelocsEnd:
  adr c0, .LCREnd
  ldr x0, [c0]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret


.align 12 

.global tt_l1_base
tt_l1_base:
.fill 4096 , 1 , 0

.global tt_l2_base
tt_l2_base:
.fill 4096 , 1 , 0

