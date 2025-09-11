/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Costin Lupu <costin.lupu@cs.pub.ro>
 *
 * Copyright (c) 2018, NEC Europe Ltd., NEC Corporation. All rights reserved.
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

#include <stdint.h>
#include <stdlib.h>
#include <uk/plat/thread.h>
#include <uk/plat/common/sw_ctx.h>
#include <uk/assert.h>
#include <uk/plat/common/tls.h>
#include <uk/plat/common/cpu.h>

static void *sw_ctx_create(struct uk_alloc *allocator, uintptr_t* sp,
				void* tlsp);
static void  sw_ctx_start(void *ctx) __noreturn;
static void  sw_ctx_switch(void *prevctx, void *nextctx);


/* Gets run when a new thread is scheduled the first time ever,
 * defined in x86_[32/64].S
 */
extern void asm_thread_starter(void);

static void *sw_ctx_create(struct uk_alloc *allocator, uintptr_t* sp,
				void* tlsp)
{
	struct sw_ctx *ctx;

	UK_ASSERT(allocator != NULL);

	ctx = arch_alloc_sw_ctx(allocator);
	if (ctx == NULL) {
		uk_pr_warn("Error allocating software context.");
		return NULL;
	}
	UK_ASSERT(__builtin_cheri_tag_get(sp) == 1);
	ctx->sp = sp;
	UK_ASSERT(__builtin_cheri_tag_get(ctx->sp) == 1);
	ctx->tlsp = tlsp;
	UK_ASSERT(__builtin_cheri_tag_get(tlsp) == 1);
	UK_ASSERT(__builtin_cheri_tag_get(ctx->tlsp) == 1);
	ctx->ip =  asm_thread_starter;
	// arch_init_extregs(ctx);

	// save_extregs(ctx);

	return ctx;
}

void test_ctx(void *ctx) {
	//uk_pr_crit("test ctx\n");
	struct sw_ctx *sw_ctx = ctx;
	volatile void* tlsp = sw_ctx->tlsp;
	volatile uintptr_t** sp = sw_ctx->sp;
	volatile void* ip = sw_ctx->ip;
	//UK_ASSERT(__builtin_cheri_tag_get(*((sp) + sizeof(uintptr_t*))) == 1);
}

extern void asm_ctx_start(uintptr_t* sp, void* ip) __noreturn;

static void sw_ctx_start(void *ctx)
{
	struct sw_ctx *sw_ctx = ctx;
	test_ctx(sw_ctx);

	UK_ASSERT(sw_ctx != NULL);

	volatile void* tlsp = sw_ctx->tlsp;
	volatile uintptr_t** sp = sw_ctx->sp;
	volatile void* ip = sw_ctx->ip;

	//UK_ASSERT(__builtin_cheri_tag_get(*((sp) + 1)) == 1);

	//uk_pr_crit("ctx sp: %p\n", sp);

	uintptr_t* func = *((sp) + 1);
	uintptr_t* arg = *((sp));

	//UK_ASSERT((uintptr_t)sp % 16 == 0);

	set_tls_pointer(tlsp);
	/* Switch stacks and run the thread */
	//asm_ctx_start(sp, ip);
	asm (
		"mov csp, %0\n"
		"mov c2, csp\n"
		"mov c0, %2\n"
		"mov c1, %1\n"
		"adrp c30, uk_sched_thread_exit\n"
		// "gctag x3, c1\n"
		// "tgcheck: cbz x3, tgcheck\n"
		"br  c1\n"
		:
		: "r"(sp), "r"(func), "r"(arg)
		: "c0", "c1", "c2"
	);

	UK_CRASH("Thread did not start.");
}

extern void asm_sw_ctx_switch(void *prevctx, void *nextctx);

static void do_switch(void *p, void *n) {
	//uk_pr_crit("Do switch\n");
		asm (
		"b crunchybit1\n"
		"restore_point1: ldp c19, c20, [c2, #32 * 6]\n"
		"msr cid_el0, c19\n"
		"msr ctpidr_el0, c20\n"
		"isb\n"
		"ldp c19, c20, [c2, #32 * 0]\n"
		"ldp c21, c22, [c2, #32 * 1]\n"
		"ldp c23, c24, [c2, #32 * 2]\n"
		"ldp c25, c26, [c2, #32 * 3]\n"
		"ldp c27, c28, [c2, #32 * 4]\n"
		"ldp c29, c30, [c2, #32 * 5]\n"

		"add csp, c2, #224\n"
		"ret c30\n"
		"crunchybit1: mov c0, %1\n"
		"mov c1, %0\n"
		"sub c2, csp, #224\n"
		"stp c19, c20, [c2, #32 * 0]\n"
		"stp c21, c22, [c2, #32 * 1]\n"
		"stp c23, c24, [c2, #32 * 2]\n"
		"stp c25, c26, [c2, #32 * 3]\n"
		"stp c27, c28, [c2, #32 * 4]\n"
		"stp c29, c30, [c2, #32 * 5]\n"
		"mrs c19, cid_el0\n"
		"mrs c20, ctpidr_el0\n"
		"stp c19, c20, [c2, #32 * 6]\n"

	/*
	 * Record the restore point for switch out thread to restore
	 * its called-saved registers in next switch to time.
	 */
		"adr c30, restore_point1\n"

	/* Save sp and restore point to previous context */
		"stp c2, c30, [c0]\n"

	/* Restire sp and restore point from next context */
		"ldp c2, c30, [c1]\n"
		"stp c2, c30, [c1]\n"
		"mov csp, c2\n"

		"ret c30\n"
		// "gctag x3, c1\n"
		// "tgcheck: cbz x3, tgcheck\n"
		:
		: "r"(n), "r"(p), "r"(asm_sw_ctx_switch)
		: "c0", "c1", "c2"
	);
}

static void sw_ctx_switch(void *prevctx, void *nextctx)
{
	//struct sw_ctx *p = prevctx;
	//struct sw_ctx *n = nextctx;

	// uk_pr_crit("prev ip: %p, next ip: %p\n", p->ip, n->ip);

	// save_extregs(p);
	// restore_extregs(n);
	// set_tls_pointer(n->tlsp);


	do_switch(prevctx, nextctx);
}

void sw_ctx_callbacks_init(struct ukplat_ctx_callbacks *ctx_cbs)
{
	UK_ASSERT(ctx_cbs != NULL);
	ctx_cbs->create_cb = sw_ctx_create;
	ctx_cbs->start_cb = sw_ctx_start;
	ctx_cbs->switch_cb = sw_ctx_switch;
}
