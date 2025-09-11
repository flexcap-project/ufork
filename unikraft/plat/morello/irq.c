/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Santiago Pagani <santiagopagani@gmail.com>
 * 			John A. Kressel <jkressel.apps@gmail.com>
 *
 * Copyright (c) 2020, NEC Laboratories Europe GmbH, NEC Corporation.
 *                     All rights reserved.
 * Copyright (c) 2022, John A. Kressel <jkressel.apps@gmail.com>
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
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <stdlib.h>
#include <uk/plat/irq.h>
#include <uk/print.h>
#include <uk/essentials.h>
#include <morello/irq.h>
#include <morello/time.h>
#include <morello/morello_info.h>
#include <arm/time.h>
#include <sfork.h>
#include <uk/alloc.h>
#include <uk/list.h>
#include <uk/plat/lcpu.h>
#include <uk/plat/common/cpu.h>
#include <uk/plat/common/irq.h>
//#include <morello/intctrl.h>
#include <uk/assert.h>
#include <errno.h>
#include <uk/bitops.h>
#include <sfork.h>

//static irq_handler_func_t irq_handlers[IRQS_MAX];

struct __a64regs exception_regs_a64;

static struct uk_alloc *allocator;

struct irq_handler {
	irq_handler_func_t func;
	void *arg;

	UK_SLIST_ENTRY(struct irq_handler) entries;
};

UK_SLIST_HEAD(irq_handler_head, struct irq_handler);
static struct irq_handler_head irq_handlers[__MAX_IRQ];

int ukplat_irq_register(unsigned long irq, irq_handler_func_t func, void *arg)
{
	struct irq_handler *h;
	unsigned long flags;

	UK_ASSERT(irq < __MAX_IRQ);
	UK_ASSERT(allocator != NULL);

	h = uk_malloc(k_alloc, sizeof(struct irq_handler));
	if (!h)
		return -ENOMEM;

	h->func = func;
	h->arg = arg;

	flags = ukplat_lcpu_save_irqf();
	UK_SLIST_INSERT_HEAD(&irq_handlers[irq], h, entries);
	ukplat_lcpu_restore_irqf(flags);

	intctrl_clear_irq(irq);
	return 0;
}

// int ukplat_irq_register(unsigned long irq, irq_handler_func_t func, void *arg __unused)
// {
// 	switch (irq) {
// 		case IRQ_ID_ARM_GENERIC_TIMER:
// 			break;
// 		default:
// 			// Unsupported IRQ
// 			uk_pr_err("ukplat_irq_register: Unsupported IRQ\n");
// 			return -1;
// 	}

// 	irq_handlers[irq] = func;
// 	return 0;
// }

static inline void ukarch_tlb_flush(void)
{
	__asm__ __volatile__(
		"	dsb	ishst\n"     /* wait for write complete */
		"	tlbi	vmalle1is\n" /* invalidate all */
        "TLBI VMALLE1\n"
        "DSB ISHST\n"
        "TLBI VMALLE1\n"
		"	dsb	ish\n"       /* wait for invalidate complete */
        "   dsb st\n"
		"	dsb sy\n"
		"	isb\n"               /* sync context */
		::: "memory");
}

void ukplat_morello_print_exception_reason(uint32_t esr, unsigned long far, unsigned long elr, uintptr_t lr) {
	int printit = 1;
	//asm("wfi\n");
	switch (esr & 0x3F) {
		case ADDR_SZ_FLT_L0:
			
			if (far)
			handle_level3_page_fault(far, elr, 0);
			else
			uk_pr_crit("Address size fault, level 0 of translation or translation table base register far %p, elr %p, pid %d\n", far, elr, PROCESS_ID());
			// for (int i = 0; i < 100000000; i++) {

			// 	asm("nop\n");
				
			// }
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ADDR_SZ_FLT_L1:
			uk_pr_crit("Address size fault, level 1\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ADDR_SZ_FLT_L2:
			uk_pr_crit("Address size fault, level 2\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ADDR_SZ_FLT_L3:
			 handle_level3_page_fault(far, elr, 2);
			 printit = 0;
			 ukarch_tlb_flush();
			uk_pr_crit("Address size fault, level 3\n");
			break;
		case TRANS_FLT_L0:
			uk_pr_crit("Translation fault, level 0\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case TRANS_FLT_L1:
			uk_pr_crit("Translation fault, level 1\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case TRANS_FLT_L2:
			uk_pr_crit("Translation fault, level 2\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case TRANS_FLT_L3:
			uk_pr_crit("Translation fault, level 3\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ACS_FLG_FLT_L1:
			uk_pr_crit("Access flag fault, level 1\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ACS_FLG_FLT_L2:
			uk_pr_crit("Access flag fault, level 2\n");
			printit = 0;
			 ukarch_tlb_flush();
			break;
		case ACS_FLG_FLT_L3:
			handle_level3_page_fault(far, elr, 2);
			printit = 0;
			//uk_pr_err("Access flag fault, level 3\n");
			break;
		case PERM_FLT_L1:
			uk_pr_err("Permission fault, level 1\n");
			break;
		case PERM_FLT_L2:
			uk_pr_err("Permission fault, level 2\n");
			break;
		case PERM_FLT_L3:
			//uk_pr_err("Permission fault, level 3, handing off to page fault handler\n");
			handle_level3_page_fault(far, elr, 0);
			printit = 0;
			break;
		case SYNC_EXT_ABRT_NOT_TTW:
			uk_pr_err("Synchronous External abort, not on translation table walk\n");
			break;
		case SYNC_EXT_ABRT_TTW_L0:
			uk_pr_err("Synchronous External abort, on translation table walk, level 0\n");
			break;
		case SYNC_EXT_ABRT_TTW_L1:
			uk_pr_err("Synchronous External abort, on translation table walk, level 1\n");
			break;
		case SYNC_EXT_ABRT_TTW_L2:
			uk_pr_err("Synchronous External abort, on translation table walk, level 2\n");
			break;
		case SYNC_EXT_ABRT_TTW_L3:
			uk_pr_err("Synchronous External abort, on translation table walk, level 3\n");
			break;
		case SYNC_PAR_ECC_ERR_MEM_ACC_NOT_TTW:
			uk_pr_err("Synchronous parity or ECC error on memory access, not on translation table walk\n");
			break;
		case SYNC_PAR_ECC_ERR_MEM_ACC_TTW_L0:
			uk_pr_err("Synchronous parity or ECC error on memory access on translation table walk, level 0\n");
			break;
		case SYNC_PAR_ECC_ERR_MEM_ACC_TTW_L1:
			uk_pr_err("Synchronous parity or ECC error on memory access on translation table walk, level 1\n");
			break;
		case SYNC_PAR_ECC_ERR_MEM_ACC_TTW_L2:
			uk_pr_err("Synchronous parity or ECC error on memory access on translation table walk, level 2\n");
			break;
		case SYNC_PAR_ECC_ERR_MEM_ACC_TTW_L3:
			uk_pr_err("Synchronous parity or ECC error on memory access on translation table walk, level 3\n");
			break;
		case CAP_TAG_FLT:
			uk_pr_err("Capability tag fault\n");
			break;
		case CAP_SEALED_FLT:
			uk_pr_err("Capability sealed fault\n");
			break;
		case CAP_BOUND_FLT:
			uk_pr_err("Capability bound fault\n");
			break;
		case CAP_PERM_FLT:
			uk_pr_err("Capability permission fault\n");
			break;
		case PG_TBL_LC_SC:
			//uk_pr_err("Page table LC or SC permission violation fault\n");
			handle_level3_page_fault(far, elr, 1);
			printit = 0;
			break;
		case TLB_CONFLICT_ABRT:
			uk_pr_err("TLB conflict abort\n");
			break;
		case UNSUPPORTED_ATOMIC_HARDWARE_UPDATE_FLT:
			uk_pr_err("Unsupported atomic hardware update fault, if the implementation includes xARMv8.1-TTHM\n");
			break;
		default:
			uk_pr_err("Exception type not handled here, I suggest you take a look at the ESR bits and then consult the ISA Manual.");
	}
	if (printit) {
		uk_pr_err("esr_el1: %lx\n", esr);
		uk_pr_err("far_el1: %p\n", far);
		uk_pr_err("elr_el1: %p\n", elr);
		uk_pr_err("lr %p\n", lr);
		uk_pr_err("pid: %d\n", PROCESS_ID());
		while(1);
	}
}

int ukplat_irq_init(struct uk_alloc *a __unused)
{
	// for (unsigned int i = 0; i < IRQS_MAX; i++) {
	// 	irq_handlers[i] = NULL;
	// }
	// // *DISABLE_BASIC_IRQS = 0xFFFFFFFF;
	// // *DISABLE_IRQS_1 = 0xFFFFFFFF;
	// // *DISABLE_IRQS_2 = 0xFFFFFFFF;
	UK_ASSERT(allocator == NULL);
	allocator = a;
	disable_irq();
	irq_vector_init();
	enable_irq();
	return 0;
}

void show_invalid_entry_message(int type)
{
	uk_pr_err("IRQ: %d\n", type);
}

void dump_registers(uintptr_t far) {
	// uk_pr_err("IRQ: %d\n", type);
	// uk_pr_err("c0 lo: 0x%x hi: 0x%x\n", regs->x0, regs->x0_hi);
	// uk_pr_err("c1 lo: 0x%x hi: 0x%x\n", regs->x1, regs->x1_hi);
	// uk_pr_err("c2 lo: 0x%x hi: 0x%x\n", regs->x2, regs->x2_hi);
	// uk_pr_err("c3 lo: 0x%x hi: 0x%x\n", regs->x3, regs->x3_hi);
	// uk_pr_err("c4 lo: 0x%x hi: 0x%x\n", regs->x4, regs->x4_hi);
	// uk_pr_err("c5 lo: 0x%x hi: 0x%x\n", regs->x5, regs->x5_hi);
	// uk_pr_err("c6 lo: 0x%x hi: 0x%x\n", regs->x6, regs->x6_hi);
	// uk_pr_err("c7 lo: 0x%x hi: 0x%x\n", regs->x7, regs->x7_hi);
	// uk_pr_err("c8 lo: 0x%x hi: 0x%x\n", regs->x8, regs->x8_hi);
	// uk_pr_err("c9 lo: 0x%x hi: 0x%x\n", regs->x9, regs->x9_hi);
	// uk_pr_err("c10 lo: 0x%x hi: 0x%x\n", regs->x10, regs->x10_hi);
	// uk_pr_err("c11 lo: 0x%x hi: 0x%x\n", regs->x11, regs->x11_hi);
	// uk_pr_err("c12 lo: 0x%x hi: 0x%x\n", regs->x12, regs->x12_hi);
	// uk_pr_err("c13 lo: 0x%x hi: 0x%x\n", regs->x13, regs->x13_hi);
	// uk_pr_err("c14 lo: 0x%x hi: 0x%x\n", regs->x14, regs->x14_hi);
	// uk_pr_err("c15 lo: 0x%x hi: 0x%x\n", regs->x15, regs->x15_hi);
	// uk_pr_err("c16 lo: 0x%x hi: 0x%x\n", regs->x16, regs->x16_hi);
	// uk_pr_err("c17 lo: 0x%x hi: 0x%x\n", regs->x17, regs->x17_hi);
	// uk_pr_err("c18 lo: 0x%x hi: 0x%x\n", regs->x18, regs->x18_hi);
	// uk_pr_err("c19 lo: 0x%x hi: 0x%x\n", regs->x19, regs->x19_hi);
	// uk_pr_err("c20 lo: 0x%x hi: 0x%x\n", regs->x20, regs->x20_hi);
	// uk_pr_err("c21 lo: 0x%x hi: 0x%x\n", regs->x21, regs->x21_hi);
	// uk_pr_err("c22 lo: 0x%x hi: 0x%x\n", regs->x22, regs->x22_hi);
	// uk_pr_err("c23 lo: 0x%x hi: 0x%x\n", regs->x23, regs->x23_hi);
	// uk_pr_err("c24 lo: 0x%x hi: 0x%x\n", regs->x24, regs->x24_hi);
	// uk_pr_err("c25 lo: 0x%x hi: 0x%x\n", regs->x25, regs->x25_hi);
	// uk_pr_err("c26 lo: 0x%x hi: 0x%x\n", regs->x26, regs->x26_hi);
	// uk_pr_err("c27 lo: 0x%x hi: 0x%x\n", regs->x27, regs->x27_hi);
	// uk_pr_err("c28 lo: 0x%x hi: 0x%x\n", regs->x28, regs->x28_hi);
	// uk_pr_err("fp lo: 0x%x hi: 0x%x\n", regs->x29, regs->x29_hi);
	// uk_pr_err("lr lo: 0x%x hi: 0x%x\n", regs->x30, regs->x30_hi);
	// uk_pr_err("sp: 0x%x\n", regs->x31);
	// uk_pr_err("esr_el1: 0x%x\n", regs->esr_el1);
	// uk_pr_err("elr_el1: 0x%x\n", regs->elr_el1);
	// uk_pr_err("spsr_el1: 0x%x\n", regs->spsr_el1);
	// uk_pr_err("cctlr_el1: 0x%x\n", regs->cctlr);
	// uk_pr_err("cpacr_el1: 0x%x\n", regs->cpacr_el1);
	uk_pr_err("far_el1: 0x%x\n", far);
	//ukplat_morello_print_exception_reason(regs->esr_el1);
}

void print_exception(uint32_t esr, uint64_t far, uintptr_t elr, uintptr_t lr) {
	ukplat_morello_print_exception_reason(esr, far, elr, lr);
}

void show_invalid_entry_message_el1_sync(uint64_t esr_el, uint64_t far_el)
{
	uk_pr_debug("ESR_EL1: %lx, FAR_EL1: %lx, SCTLR_EL1:%lx\n", esr_el, far_el, get_sctlr_el1());
}

// void ukplat_irq_handle(void)
// {

// 	// if ((get_el0(cntv_ctl) & GT_TIMER_IRQ_STATUS) && irq_handlers[IRQ_ID_ARM_GENERIC_TIMER]) {
// 	// 	irq_handlers[IRQ_ID_ARM_GENERIC_TIMER](NULL);
// 	// 	return;
// 	// }

// 	uk_pr_err("ukplat_irq_handle: Unhandled IRQ\n");
// 	while(1);
// }

void _ukplat_irq_handle(struct __regs *regs, unsigned long irq)
{

	struct irq_handler_head h;
	//uk_pr_err("IRQ %d\n", irq);
	//while(1);
	// UK_SLIST_FOREACH(h, &irq_handlers[irq], entries) {
	// 	/* TODO define platform wise macro for timer IRQ number */
	// 	if (irq != 0)
	// 		/* IRQ 0 is reserved for a timer, responsible to
	// 		 * wake up cpu from halt, so it can check if
	// 		 * it has something to do. Effectively it is OS ticks.
	// 		 *
	// 		 * If interrupt comes not from the timer, the
	// 		 * chances are some work have just
	// 		 * arrived. Let's kick the scheduler out of
	// 		 * the halting loop, and let it take care of
	// 		 * that work.
	// 		 */
	// 		//__uk_test_and_set_bit(0, &sched_have_pending_events);

	// 	if (h->func(h->arg) == 1)
	// 		goto exit_ack;
	// }

	// h = irq_handlers[irq];
	// if (h.slh_first->func(h.slh_first->arg) == 1)
	// 		goto exit_ack;
	// if (irq == 7) {
	// 	handle_irq();
	// }
	// if (irq == 7)
	handle_irq();
	// else if (irq == 9)
	// wait_proc();
	/*
	 * Acknowledge interrupts even in the case when there was no handler for
	 * it. We do this to (1) compensate potential spurious interrupts of
	 * devices, and (2) to minimize impact on drivers that share one
	 * interrupt line that would then stay disabled.
	 */
	//uk_pr_err("Unhandled irq=%lu\n", irq);

exit_ack:
	intctrl_ack_irq(irq);
}

// void _ukplat_irq_handle(unsigned long irq)
// {

// 	ukplat_irq_handle(irq);
// }
