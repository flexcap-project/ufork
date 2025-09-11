/*
 * MIT License
 *
 * Copyright (c) 2018, Sergey Matyukevich
 *           (c) 2020, Santiago Pagani <santiagopagani@gmail.com>
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

#include <morello/entry.h>

	.macro handle_invalid_entry type
//ldr x0, =ptb1
//msr ttbr0_el1, x0
//isb
//	msr SPSel, #1
	stp c0, c1, [csp, #-32]!
	stp c2, c3, [csp, #-32]!
	stp c4, c5, [csp, #-32]!
	stp c6, c7, [csp, #-32]!
	stp c8, c9, [csp, #-32]!
	stp c10, c11, [csp, #-32]!
	stp c12, c13, [csp, #-32]!
	stp c14, c15, [csp, #-32]!
	stp c16, c17, [csp, #-32]!
	stp c18, c19, [csp, #-32]!
	stp c20, c21, [csp, #-32]!
	stp c22, c23, [csp, #-32]!
	stp c24, c25, [csp, #-32]!
	stp c26, c27, [csp, #-32]!
	stp c28, c29, [csp, #-32]!
	str c30, [csp, #-16]!
	mrs x0, SPSR_EL1
	str x0, [csp, #-16]!
	mrs c0, CELR_EL1
	str c0, [csp, #-16]!

	mrs x1, far_el1
	mrs x2, elr_el1
	mrs x0, esr_el1
	mov x3, lr
	bl print_exception
	ldr c0, [csp], #16
	msr CELR_EL1, c0
	ldr x0, [csp], #16
	msr SPSR_EL1, x0
	ldr c30, [csp], #16
	ldp c28, c29, [csp], #32
	ldp c26, c27, [csp], #32
	ldp c24, c25, [csp], #32
	ldp c22, c23, [csp], #32
	ldp c20, c21, [csp], #32
	ldp c18, c19, [csp], #32
	ldp c16, c17, [csp], #32
	ldp c14, c15, [csp], #32
	ldp c12, c13, [csp], #32
	ldp c10, c11, [csp], #32
	ldp c8, c9, [csp], #32
	ldp c6, c7, [csp], #32
	ldp c4, c5, [csp], #32
	ldp c2, c3, [csp], #32
	ldp c0, c1, [csp], #32
//	   ldr x0, =ptb
//	msr ttbr0_el1, x0
//	isb
//	dsb st
//	msr SPSel, #0
	eret
//	bl	dump_registers
//	wfi
//	msr             SPSel, #0
//	mov x17, sp
//	msr             SPSel, #1
//	nop
//	nop
//	nop
//	nop
//	mov sp, x17
//	mrs x17, spsr_el1
//	eor x17, x17, #1
//	msr spsr_el1, x17
//	isb
//	kernel_exit
//	eret
	.endm

	.macro	ventry	label
	.align	7
	b	\label
	.endm

	.macro	kernel_entry
	; sub	csp, csp, #S_FRAME_SIZE
	; stp	c0, c1, [csp, #32 * 0]
	; stp	c2, c3, [csp, #32 * 1]
	; stp	c4, c5, [csp, #32 * 2]
	; stp	c6, c7, [csp, #32 * 3]
	; stp	c8, c9, [csp, #32 * 4]
	; stp	c10, c11, [csp, #32 * 5]
	; stp	c12, c13, [csp, #32 * 6]
	; stp	c14, c15, [csp, #32 * 7]
	; stp	c16, c17, [csp, #32 * 8]
	; stp	c18, c19, [csp, #32 * 9]
	; stp	c20, c21, [csp, #32 * 10]
	; stp	c22, c23, [csp, #32 * 11]
	; stp	c24, c25, [csp, #32 * 12]
	; stp	c26, c27, [csp, #32 * 13]
	; stp	c28, c29, [csp, #32 * 14]
	; str	c30, [csp, #32 * 15] 
	.endm

	.macro	kernel_exit
	; ldp	c0, c1, [csp, #32 * 0]
	; ldp	c2, c3, [csp, #32 * 1]
	; ldp	c4, c5, [csp, #32 * 2]
	; ldp	c6, c7, [csp, #32 * 3]
	; ldp	c8, c9, [csp, #32 * 4]
	; ldp	c10, c11, [csp, #32 * 5]
	; ldp	c12, c13, [csp, #32 * 6]
	; ldp	c14, c15, [csp, #32 * 7]
	; ldp	c16, c17, [csp, #32 * 8]
	; ldp	c18, c19, [csp, #32 * 9]
	; ldp	c20, c21, [csp, #32 * 10]
	; ldp	c22, c23, [csp, #32 * 11]
	; ldp	c24, c25, [csp, #32 * 12]
	; ldp	c26, c27, [csp, #32 * 13]
	; ldp	c28, c29, [csp, #32 * 14]
	; ldr	c30, [csp, #32 * 15] 
	; add	csp, csp, #S_FRAME_SIZE		
	; eret
	.endm


/*
 * Exception vectors.
 */
.align	11
.globl vectors_el3
vectors_el3:
	ventry	sync_invalid_el3t			// Synchronous EL3t
	ventry	irq_invalid_el3t			// IRQ EL3t
	ventry	fiq_invalid_el3t			// FIQ EL3t
	ventry	error_invalid_el3t			// Error EL3t

	ventry	sync_invalid_el3h			// Synchronous EL3h
	ventry	irq_invalid_el3h			// IRQ EL3h
	ventry	fiq_invalid_el3h			// FIQ EL3h
	ventry	error_invalid_el3h			// Error EL3h

	ventry	sync_invalid_el2_64			// Synchronous 64-bit EL2
	ventry	irq_invalid_el2_64			// IRQ 64-bit EL2
	ventry	fiq_invalid_el2_64			// FIQ 64-bit EL2
	ventry	error_invalid_el2_64		// Error 64-bit EL2

	ventry	sync_invalid_el2_32			// Synchronous 32-bit EL2
	ventry	irq_invalid_el2_32			// IRQ 32-bit EL2
	ventry	fiq_invalid_el2_32			// FIQ 32-bit EL2
	ventry	error_invalid_el2_32		// Error 32-bit EL2

.align	11
.globl vectors_el2
vectors_el2:
	ventry	sync_invalid_el2t			// Synchronous EL2t
	ventry	irq_invalid_el2t			// IRQ EL2t
	ventry	fiq_invalid_el2t			// FIQ EL2t
	ventry	error_invalid_el2t			// Error EL2t

	ventry	sync_invalid_el2h			// Synchronous EL2h
	ventry	irq_invalid_el2h			// IRQ EL2h
	ventry	fiq_invalid_el2h			// FIQ EL2h
	ventry	error_invalid_el2h			// Error EL2h

	ventry	sync_invalid_el1_64			// Synchronous 64-bit EL1
	ventry	irq_invalid_el1_64			// IRQ 64-bit EL1
	ventry	fiq_invalid_el1_64			// FIQ 64-bit EL1
	ventry	error_invalid_el1_64		// Error 64-bit EL1

	ventry	sync_invalid_el1_32			// Synchronous 32-bit EL1
	ventry	irq_invalid_el1_32			// IRQ 32-bit EL1
	ventry	fiq_invalid_el1_32			// FIQ 32-bit EL1
	ventry	error_invalid_el1_32		// Error 32-bit EL1

.align	11
.globl vectors_el1 
vectors_el1:
	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	ventry	el1_sync					// Synchronous EL1h
	ventry	el1_irq						// IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
	ventry	irq_invalid_el0_64			// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64		// Error 64-bit EL0

	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0

sync_invalid_el3t:
	handle_invalid_entry  SYNC_INVALID_EL3t

irq_invalid_el3t:
	handle_invalid_entry  IRQ_INVALID_EL3t

fiq_invalid_el3t:
	handle_invalid_entry  FIQ_INVALID_EL3t

error_invalid_el3t:
	handle_invalid_entry  ERROR_INVALID_EL3t

sync_invalid_el3h:
	handle_invalid_entry  SYNC_INVALID_EL3h

irq_invalid_el3h:
	handle_invalid_entry  IRQ_INVALID_EL3h

fiq_invalid_el3h:
	handle_invalid_entry  FIQ_INVALID_EL3h

error_invalid_el3h:
	handle_invalid_entry  ERROR_INVALID_EL3h

sync_invalid_el2_64:
	handle_invalid_entry  SYNC_INVALID_EL2_64

irq_invalid_el2_64:
	handle_invalid_entry  IRQ_INVALID_EL2_64

fiq_invalid_el2_64:
	handle_invalid_entry  FIQ_INVALID_EL2_64

error_invalid_el2_64:
	handle_invalid_entry  ERROR_INVALID_EL2_64

sync_invalid_el2_32:
	handle_invalid_entry  SYNC_INVALID_EL2_32

irq_invalid_el2_32:
	handle_invalid_entry  IRQ_INVALID_EL2_32

fiq_invalid_el2_32:
	handle_invalid_entry  FIQ_INVALID_EL2_32

error_invalid_el2_32:
	handle_invalid_entry  ERROR_INVALID_EL2_32

sync_invalid_el2t:
	handle_invalid_entry  SYNC_INVALID_EL2t

irq_invalid_el2t:
	handle_invalid_entry  IRQ_INVALID_EL2t

fiq_invalid_el2t:
	handle_invalid_entry  FIQ_INVALID_EL2t

error_invalid_el2t:
	handle_invalid_entry  ERROR_INVALID_EL2t

sync_invalid_el2h:
	handle_invalid_entry  SYNC_INVALID_EL2h

irq_invalid_el2h:
	handle_invalid_entry  IRQ_INVALID_EL2h

fiq_invalid_el2h:
	handle_invalid_entry  FIQ_INVALID_EL2h

error_invalid_el2h:
	handle_invalid_entry  ERROR_INVALID_EL2h

sync_invalid_el1_64:
	handle_invalid_entry  SYNC_INVALID_EL1_64

irq_invalid_el1_64:
	handle_invalid_entry  IRQ_INVALID_EL1_64

fiq_invalid_el1_64:
	handle_invalid_entry  FIQ_INVALID_EL1_64

error_invalid_el1_64:
	handle_invalid_entry  ERROR_INVALID_EL1_64

sync_invalid_el1_32:
	handle_invalid_entry  SYNC_INVALID_EL1_32

irq_invalid_el1_32:
	handle_invalid_entry  IRQ_INVALID_EL1_32

fiq_invalid_el1_32:
	handle_invalid_entry  FIQ_INVALID_EL1_32

error_invalid_el1_32:
	handle_invalid_entry  ERROR_INVALID_EL1_32

sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  SYNC_INVALID_EL1h

irq_invalid_el1h:
	handle_invalid_entry  IRQ_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32

el1_sync:
	handle_invalid_entry  SYNC_INVALID_EL1h
//	kernel_entry
//	mrs	x0, ESR_EL1
//	mrs	x1, FAR_EL1
//	bl	show_invalid_entry_message_el1_sync
//	b	err_hang

el1_irq:
	kernel_entry 
	bl	_ukplat_irq_handle
	kernel_exit

maybezero:
	b maybezero

.globl err_hang
err_hang: b err_hang


