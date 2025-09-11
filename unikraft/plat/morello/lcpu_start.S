/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Răzvan Vîrtan <virtanrazvan@gmail.com>
 *          Justin He     <justin.he@arm.com>
 *
 * Copyright (c) 2021, Arm Ltd. All rights reserved.
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

 #include <uk/asm.h>
 #include <uk/plat/common/lcpu.h>

.text
/* void _lcpu_start(lcpu *cpu) */
ENTRY(lcpu_start)
	/* save x0 */
	mov	x9, x0

	/* Disable interrupts */
	msr	daifset, #2

	// 	ldr        x0, =(3 << 18) //CEN (bits 18 and 19) are 1 and TC (bit 9) is 0
	// msr        cptr_el2, x0

	ldr x1, =(3 << 20) //fp
	ldr x2, =(3 << 18) // CEN (bits 18 and 19) set to 1
	orr x0, x1, x0
	orr x0, x0, x2
	msr		cpacr_el1, x0 //enable fp and capabilities

	isb


	/* Set the context id */
	msr	contextidr_el1, xzr
	/* Setup exception vector table address before enable MMU */
	ldr      x0, =vectors_el1
	cvtp     c0, x0
  	msr      cvbar_el1, c0

	// ldr		x0, =(0x05)
	// msr		spsr_el2, x0

	ldr		x0, =(0x05)
	msr		spsr_el1, x0

	// ldr		x0, =(el1_entry_lcpu)
	// cvtp    c0, x0
	// msr		celr_el2, c0

	mrs     x0, cctlr_el1
	orr x0, x0, #(0x1 << 0)
	orr x0, x0, #(0x1 << 5)
	bic x0, x0, #(0x1 << 7) //don't want return caps to be sealed, you may want this behaviour enabled though

	msr cctlr_el1, x0

ldr x0, =tt_l1_base //get the base page table
	msr ttbr0_el1, x0 

	// ldr x0, =_pagetables1 //get the base page table
	// msr ttbr1_el1, x0 

//	eret  //jump down to el1


el1_entry_lcpu:
	mov   x0, #0x000000000000FF44
	msr   mair_el1, x0



	// Set up TCR_EL1
	// ---------------
	mov   x0, #0x19             // T0SZ=0b011001 Limits VA space to 39 bits
	orr   x0, x0, #(0x1 << 8)   // IGRN0=0b01  Walks to TTBR0 are Inner WB/WA
	orr   x0, x0, #(0x1 << 10)  // OGRN0=0b01  Walks to TTBR0 are Outer WB/WA
	orr   x0, x0, #(0x3 << 12)  // SH0=0b11   Inner Shareable
	orr   x0, x0, #(0x1 << 23)  // EPD1=0b1   Disable table walks from TTBR1
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

	msr   tcr_el1, x0
	/* Enable the mmu */
	 	mov      x0, #(1 << 0)                     // M=1           Enable the stage 1 MMU
  	orr      x0, x0, #(1 << 2)                 // C=1           Enable data and unified caches
  	orr      x0, x0, #(1 << 12)                // I=1           Enable instruction fetches to allocate into unified caches
                                             // A=0           Strict alignment checking disabled
                                             // SA=0          Stack alignment checking disabled
                                             // WXN=0         Write permission does not imply XN
                                             // EE=0          EL3 data accesses are little endian
	msr      SCTLR_EL1, x0
	isb

	/* restore x0 */
	mov	x0, x9
	cvtp c0, x0

	 	msr SPSel, #0
	// ldr x1, =__stack_end1
	// and x1, x1, #-16
	// cvtp c1, x1
	// mov csp, c1
	// msr SPSel, #1
	// mov csp, c1
	// msr SPSel, #0



	/* Load the stack pointer */
	ldr	x9, [c0, #LCPU_STACKP_OFFSET]
	and             x9, x9, #-16
	cvtp            c9, x9
	mov             csp, c9
	msr SPSel, #1
	mov csp, c9
	msr SPSel, #0

	/* Load the entry address and jump to it */
	ldr	x9, [c0, #LCPU_ENTRY_OFFSET]
	cvtp c9, x9
	//wfi
	br	c9
	// ldr x4, =test_function
	// cvtp c4, x4
	// br c4
END(lcpu_start)
