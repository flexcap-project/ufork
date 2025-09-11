/* SPDX-License-Identifier: MIT */
/*
 * Authors: Rolf Neugebauer
 *          Grzegorz Milos
 *          Costin Lupu <costin.lupu@cs.pub.ro>
 *
 * Copyright (c) 2003-2005, Intel Research Cambridge
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
 * Thread definitions
 * Ported from Mini-OS
 */
#include <flexos/isolation.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <uk/plat/config.h>
#include <uk/plat/time.h>
#include <uk/thread.h>
#include <uk/sched.h>
#include <uk/page.h>
#include <uk/wait.h>
#include <uk/print.h>
#include <uk/assert.h>
#include <uk/arch/tls.h>

#if CONFIG_LIBUKSIGNAL
#include <uk/uk_signal.h>
#endif

#if CONFIG_LIBFLEXOS_INTELPKU
#include <uk/wait_types.h>
struct uk_waitq_entry wq_entries[32];

static int uk_num_threads = 0;
#endif /* CONFIG_LIBFLEXOS_INTELPKU */

#if CONFIG_LIBFLEXOS_VMEPT
static int uk_num_threads = 0;
#endif /* CONFIG_LIBFLEXOS_VMEPT */

#if CONFIG_LIBFLEXOS_MORELLO
static int uk_num_threads = 10;
#endif /* CONFIG_LIBFLEXOS_MORELLO */

/* Pushes the specified value onto the stack of the specified thread */
static void stack_push(uint64_t **sp, void* value)
{
	*sp -= sizeof(void*);
	(*sp) = (uintptr_t*)value;
	//asm("nop\n nop\n nop\n");
	//UK_ASSERT(__builtin_cheri_tag_get(value) == 1);
	//UK_ASSERT(__builtin_cheri_tag_get(((uintptr_t *)*sp)) == 1);
}

static void init_sp(uintptr_t **sp, char *stack,
		void (*function)(void *), void *data)
{
	(*sp) = (uintptr_t*)stack + STACK_SIZE;

#if defined(__X86_64__)
	/* Must ensure that (%rsp + 8) is 16-byte aligned
	 * at the start of thread_starter.
	 */
	stack_push(sp, 0);
#endif
	//UK_ASSERT(__builtin_cheri_tag_get(function) == 1);
	//UK_ASSERT(__builtin_cheri_tag_get(data) == 1);
	//stack_push(sp, function);
	//stack_push(sp, data);
	*sp -= 2 * sizeof(uint64_t *);
	UK_ASSERT((uint64_t)(*sp) % 16 == 0);
	*((uintptr_t*)*sp) = (uintptr_t*)data;
	//(*sp)[1] = function;
}

#ifdef CONFIG_LIBNEWLIBC
static void reent_init(struct _reent *reent)
{
	_REENT_INIT_PTR(reent);
#if 0
	/* TODO initialize basic signal handling */
	_init_signal_r(myreent);
#endif
}

struct _reent *__getreent_kernel(void)
{
	struct _reent *_reent;
	struct uk_sched *s = uk_sched_get_default();

	if (!s || !uk_sched_started(s))
		_reent = _impure_ptr;
	else
		_reent = uk_thread_current()->reent;

	return _reent;
}

struct _reent *__getreent(void)__attribute__((section(".app_libs_text"))){
	if (sfork_syscalls.rent)
		return sfork_syscalls.rent();
	else 
		return __getreent_kernel();
}
#endif /* CONFIG_LIBNEWLIBC */

#if CONFIG_LIBFLEXOS_GATE_INTELPKU_PRIVATE_STACKS || CONFIG_LIBFLEXOS_MORELLO
#define SET_TSB(sp_comp, key) 						\
do {									\
	tsb_comp ## key[thread->tid].sp = (sp_comp);			\
	tsb_comp ## key[thread->tid].bp = (sp_comp);			\
} while (0)
#else /* CONFIG_LIBFLEXOS_GATE_INTELPKU_PRIVATE_STACKS */
/* do nothing without PKU private stacks */
//#define SET_TSB(sp_comp, key)
#endif /* CONFIG_LIBFLEXOS_GATE_INTELPKU_PRIVATE_STACKS */

#if CONFIG_LIBFLEXOS_INTELPKU || CONFIG_LIBFLEXOS_MORELLO
#define SET_TID_PAGE(stack_comp) 					\
do {									\
	*( (stack_comp + 1) + (round_pgup(((unsigned long) stack_comp + 1))) - ((unsigned long) stack_comp + 1) ) = thread->tid;		\
} while (0)
#else
#define SET_TID_PAGE(sp)
#endif /* CONFIG_LIBFLEXOS_INTELPKU */

#define SETUP_STACK(stack_comp, key, a1, a2, sp) 			\
do {									\
	if ((stack_comp)) {						\
		/* Save pointer to the thread on the stack to get */	\
		/* current thread. */					\
		/* FIXME FLEXOS PKU in the future this page should be */\
		/* protected with the permissions of the scheduler */	\
		/* so that it can't be subverted by a malicious */	\
		/* compartment */					\
		* ( stack_comp) = thread;				\
		UK_ASSERT((uintptr_t)*(stack_comp)%16 == 0);	\
		UK_ASSERT(__builtin_cheri_tag_get(*stack_comp) == 1);	\
	}								\
									\
} while (0)

/* This is a copy of uk_thread_init without manipulations of the PKRU,
 * for the exact same reasons that we made a copy of uk_sched_thread_create.
 */
int uk_thread_init_main(struct uk_thread *thread,
		struct ukplat_ctx_callbacks *cbs, struct uk_alloc *allocator,
		const char *name, uintptr_t **stack

,
		void *tls, void (*function)(void *), void *arg)
{
	uintptr_t** sp;

	UK_ASSERT(thread != NULL);
	UK_ASSERT(stack != NULL);
	UK_ASSERT(!have_tls_area() || tls != NULL);

#if CONFIG_LIBFLEXOS_INTELPKU
	thread->tid = uk_num_threads++;
#endif /* CONFIG_LIBFLEXOS_INTELPKU */
#if CONFIG_LIBFLEXOS_VMEPT
	// FIXME: do this properly, this is terrible
	thread->tid = uk_num_threads++;
	thread->ctrl = NULL;
#endif /* CONFIG_LIBFLEXOS_VMEPT */
#if CONFIG_LIBFLEXOS_MORELLO
	thread->tid = uk_num_threads++;
#endif

	SETUP_STACK(stack, 0, function, arg, sp);
	//uk_pr_crit("stack addr %p\n", stack);
	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);
	// stack = __builtin_cheri_address_set(stack, (uintptr_t)stack);
	//uk_pr_crit("stack base %p\n", __builtin_cheri_base_get(stack));
	//function(arg);
	//sp = ((uintptr_t*)stack) + (STACK_SIZE - sizeof(uintptr_t*));
	sp = __builtin_cheri_bounds_set(stack, STACK_SIZE);
	sp = __builtin_cheri_offset_set(sp, STACK_SIZE);
	
	//uk_pr_crit("stack %p, sp base %p\n", sp, __builtin_cheri_base_get(sp));
	sp -= 1;
	*sp = (uintptr_t*)function;
	UK_ASSERT(__builtin_cheri_tag_get(sp) == 1);
	UK_ASSERT(__builtin_cheri_tag_get(*sp) == 1);
	sp -= 1;
	*sp = (uintptr_t*)arg;
	UK_ASSERT(__builtin_cheri_tag_get(*((sp) + 1)) == 1);

	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);

	/* The toolchain is going to insert a number of calls to
	 * SETUP_STACK depending on the number of compartments, e.g.,
	 * SETUP_STACK(stack_comp1, 1, NULL, NULL); */
// 	unsigned long sp1;
// SETUP_STACK(stack_comp1, 1, NULL, NULL, sp1);
	asm("nop\n");

	/* Call platform specific setup. */
	UK_ASSERT(__builtin_cheri_tag_get(ukarch_tls_pointer(tls)) == 1);
	 thread->ctx = ukplat_thread_ctx_create(cbs, allocator, sp,
	 		ukarch_tls_pointer(tls));
	 			if (!allocator) {
	 	}
	 if (thread->ctx == NULL)
	 	return -1;

	thread->name = name;
	thread->stack = stack;

	thread->tls = tls;

	/* Not runnable, not exited, not sleeping */
	thread->flags = 0;
	thread->wakeup_time = 0LL;
	thread->detached = false;
	uk_waitq_init(&thread->waiting_threads);
	thread->sched = NULL;
	thread->prv = NULL;
	//UK_ASSERT(__builtin_cheri_tag_get(*((sp) + sizeof(uintptr_t*))) == 1);
	// FIXME
	//thread->reent = flexos_malloc_whitelist(sizeof(struct _reent), libc);
	//thread->reent = malloc(sizeof(struct _reent));
#ifdef CONFIG_LIBNEWLIBC
	thread->reent = uk_malloc(flexos_shared_alloc,sizeof(struct _reent));
	if (!thread->reent) {
		flexos_nop_gate(0, 0, uk_pr_crit,
				FLEXOS_SHARED_LITERAL("Could not allocate reent!"));
		return -1;
	}
#endif


#ifdef CONFIG_LIBNEWLIBC
	reent_init(thread->reent);
#endif
#if CONFIG_LIBUKSIGNAL
	asm("nop\n nop\n");
	thread->signals_container = uk_malloc(flexos_shared_alloc,
					      sizeof(struct uk_thread_sig));
	uk_thread_sig_init(thread->signals_container);
//	asm("yello: b yello\n");
#endif

	uk_pr_info("Thread \"%s\": pointer: %p, stack: %p - %p, tls: %p\n",
		   name, thread, stack, (void *) ((uintptr_t) stack + STACK_SIZE), tls);

	return 0;
}

int uk_thread_init(struct uk_thread *thread,
		struct ukplat_ctx_callbacks *cbs, struct uk_alloc *allocator,
		const char *name, uintptr_t **stack

,
		void *tls, void (*function)(void *), void *arg)
{
	uintptr_t** sp;

	UK_ASSERT(thread != NULL);
	UK_ASSERT(stack != NULL);
	UK_ASSERT(!have_tls_area() || tls != NULL);
thread->tid = uk_num_threads++;

	SETUP_STACK(stack, 0, function, arg, sp);

	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);
	// stack = __builtin_cheri_address_set(stack, (uintptr_t)stack);
	//uk_pr_crit("stack base %p\n", __builtin_cheri_base_get(stack));
	//function(arg);
	//sp = ((uintptr_t*)stack) + (STACK_SIZE - sizeof(uintptr_t*));
	sp = __builtin_cheri_bounds_set(stack, STACK_SIZE);
	//uk_pr_crit("initialising stack 1 %p\n", sp);
	sp = __builtin_cheri_offset_set(sp, STACK_SIZE);
	//uk_pr_crit("initialising stack 2 %p\n", sp);
	
	//uk_pr_crit("stack %p, sp base %p\n", sp, __builtin_cheri_base_get(sp));
	sp -= 1;
	*sp = (uintptr_t*)function;
	UK_ASSERT(__builtin_cheri_tag_get(sp) == 1);
	UK_ASSERT(__builtin_cheri_tag_get(*sp) == 1);
	sp -= 1;
	*sp = (uintptr_t*)arg;
	UK_ASSERT(__builtin_cheri_tag_get(*((sp) + 1)) == 1);

	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);

	//uk_pr_crit("initialised stack %p, other one %p\n", sp, stack);

	/* The toolchain is going to insert a number of calls to
	 * SETUP_STACK depending on the number of compartments, e.g.,
	 * SETUP_STACK(stack_comp1, 1, NULL, NULL); */
// 	unsigned long sp1;
// SETUP_STACK(stack_comp1, 1, NULL, NULL, sp1);



	/* Call platform specific setup. */
	thread->ctx = ukplat_thread_ctx_create(cbs, allocator, sp,
			ukarch_tls_pointer(tls));

	if (thread->ctx == NULL)
		return -1;

	thread->name = name;
	thread->stack = stack;

	thread->tls = tls;

	/* Not runnable, not exited, not sleeping */
	thread->flags = 0;
	thread->wakeup_time = 0LL;
	thread->detached = false;
	uk_waitq_init(&thread->waiting_threads);
	thread->sched = NULL;
	thread->prv = NULL;

	// FIXME
#ifdef CONFIG_LIBNEWLIBC
#if CONFIG_LIBFLEXOS_VMEPT
	thread->reent = malloc(sizeof(struct _reent));
#else
	thread->reent = uk_malloc(flexos_shared_alloc, sizeof(struct _reent));
#endif
	if (!thread->reent) {
		flexos_nop_gate(0, 0, uk_pr_crit,
				FLEXOS_SHARED_LITERAL("Could not allocate reent!"));
		return -1;
	}
#endif

#ifdef CONFIG_LIBNEWLIBC
	reent_init(thread->reent);
#endif
#if CONFIG_LIBUKSIGNAL
	thread->signals_container = uk_malloc(flexos_shared_alloc,
					      sizeof(struct uk_thread_sig));
	uk_thread_sig_init(thread->signals_container);
#endif

	flexos_nop_gate(0, 0, uk_pr_info,
			FLEXOS_SHARED_LITERAL("Thread \"%s\": pointer: %p, stack: %p - %p, tls: %p\n"),
			name, thread, stack,
			(void *)((uintptr_t)stack + STACK_SIZE), tls);

	return 0;
}




int uk_thread_init_process(struct uk_thread *thread,
		struct ukplat_ctx_callbacks *cbs, struct uk_alloc *allocator,
		const char *name, uintptr_t **stack

,
		void *tls, void (*function)(void *), void *arg, void* psp)
{
	uintptr_t** sp;

	UK_ASSERT(thread != NULL);
	UK_ASSERT(stack != NULL);
	UK_ASSERT(!have_tls_area() || tls != NULL);
thread->tid = uk_num_threads++;

	SETUP_STACK(stack, 0, function, arg, sp);

	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);
	// stack = __builtin_cheri_address_set(stack, (uintptr_t)stack);
	//uk_pr_crit("stack base %p\n", __builtin_cheri_base_get(stack));
	//function(arg);
	//sp = ((uintptr_t*)stack) + (STACK_SIZE - sizeof(uintptr_t*));
	sp = __builtin_cheri_bounds_set(stack, STACK_SIZE);
	// *sp = (unsigned long*)*sp;
	//uk_pr_crit("initialising stack 1 %p\n", sp);
	// AK TODO: may need to revisit this, with enough processes we may run out of stack space
	sp = __builtin_cheri_offset_set(sp, STACK_SIZE-(STACK_SIZE-(unsigned long)psp&0xFFF));
	//uk_pr_crit("initialising stack 2 %p\n", sp);
	
	//uk_pr_crit("stack %p, sp base %p\n", sp, __builtin_cheri_base_get(sp));
	sp -= 1;
	*sp = (uintptr_t*)function;
	UK_ASSERT(__builtin_cheri_tag_get(sp) == 1);
	UK_ASSERT(__builtin_cheri_tag_get(*sp) == 1);
	//uk_pr_crit("stored function %p on stack at %p, actual %p\n", function, sp, *sp);
	sp -= 1;
	*sp = (uintptr_t*)arg;
	//uk_pr_crit("stored function %p on stack at %p, actual %p\n", function, sp, *(sp+1));
	UK_ASSERT(__builtin_cheri_tag_get(*((sp) + 1)) == 1);

	UK_ASSERT(__builtin_cheri_tag_get(*stack) == 1);

	//uk_pr_crit("initialised stack %p, other one %p\n", sp, stack);

	/* The toolchain is going to insert a number of calls to
	 * SETUP_STACK depending on the number of compartments, e.g.,
	 * SETUP_STACK(stack_comp1, 1, NULL, NULL); */
// 	unsigned long sp1;
// SETUP_STACK(stack_comp1, 1, NULL, NULL, sp1);



	/* Call platform specific setup. */
	thread->ctx = ukplat_thread_ctx_create(cbs, allocator, sp,
			ukarch_tls_pointer(tls));

	if (thread->ctx == NULL)
		return -1;

	thread->name = name;
	thread->stack = stack;

	thread->tls = tls;

	/* Not runnable, not exited, not sleeping */
	thread->flags = 0;
	thread->wakeup_time = 0LL;
	thread->detached = false;
	uk_waitq_init(&thread->waiting_threads);
	thread->sched = NULL;
	thread->prv = NULL;

	// FIXME
#ifdef CONFIG_LIBNEWLIBC
#if CONFIG_LIBFLEXOS_VMEPT
	thread->reent = malloc(sizeof(struct _reent));
#else
	thread->reent = uk_malloc(flexos_shared_alloc, sizeof(struct _reent));
#endif
	if (!thread->reent) {
		flexos_nop_gate(0, 0, uk_pr_crit,
				FLEXOS_SHARED_LITERAL("Could not allocate reent!"));
		return -1;
	}
#endif

#ifdef CONFIG_LIBNEWLIBC
	reent_init(thread->reent);
#endif
#if CONFIG_LIBUKSIGNAL
	thread->signals_container = uk_malloc(flexos_shared_alloc,
					      sizeof(struct uk_thread_sig));
	uk_thread_sig_init(thread->signals_container);
#endif

	flexos_nop_gate(0, 0, uk_pr_info,
			FLEXOS_SHARED_LITERAL("Thread \"%s\": pointer: %p, stack: %p - %p, tls: %p\n"),
			name, thread, stack,
			(void *)((uintptr_t)stack + STACK_SIZE), tls);

	return 0;
}




void uk_thread_fini(struct uk_thread *thread, struct uk_alloc *allocator)
{
	UK_ASSERT(thread != NULL);
#if CONFIG_LIBUKSIGNAL
	uk_thread_sig_uninit(thread->signals_container);
#endif
	ukplat_thread_ctx_destroy(allocator, thread->ctx);
}

void uk_thread_inherit_signal_mask_kernel(struct uk_thread *thread)
{
	thread->signals_container->mask = uk_thread_current()->signals_container->mask;
}

void uk_thread_inherit_signal_mask(struct uk_thread *thread)__attribute__((section(".app_libs_text"))){
	if (sfork_syscalls.thread_inherit_signal_mask)
		return sfork_syscalls.thread_inherit_signal_mask(thread);
	else 
		return uk_thread_inherit_signal_mask_kernel(thread);
}

static void uk_thread_block_until(struct uk_thread *thread, __snsec until)
{
	unsigned long flags;

	flags = ukplat_lcpu_save_irqf();
	thread->wakeup_time = until;
	clear_runnable(thread);
	uk_sched_thread_blocked(thread);
	ukplat_lcpu_restore_irqf(flags);
}

void uk_thread_block_timeout_kernel(struct uk_thread *thread, __nsec nsec)
{
	__snsec until = (__snsec) ukplat_monotonic_clock() + nsec;
	//uk_pr_crit("thread struct %p\n", thread);
	uk_thread_block_until(thread, until);
}

void uk_thread_block_timeout(struct uk_thread *thread, __nsec nsec)__attribute__((section(".app_libs_text"))){
	if (sfork_syscalls.tblockt)
		return sfork_syscalls.tblockt(thread, nsec);
	else 
		return uk_thread_block_timeout_kernel(thread, nsec);
}

void uk_thread_block(struct uk_thread *thread)
{
	uk_thread_block_until(thread, 0LL);
}

void uk_thread_wake(struct uk_thread *thread)
{
	unsigned long flags;
	if (!thread || !thread->sched)
		return;

	flags = ukplat_lcpu_save_irqf();
	if (!is_runnable(thread)) {
		uk_sched_thread_woken(thread->sched, thread);
		thread->wakeup_time = 0LL;
		set_runnable(thread);
	}
	ukplat_lcpu_restore_irqf(flags);
}

void uk_thread_exit(struct uk_thread *thread)
{
	UK_ASSERT(thread);

	set_exited(thread);

	if (!thread->detached)
		uk_waitq_wake_up(&thread->waiting_threads);

	uk_pr_debug("Thread \"%s\" exited.\n", thread->name);
}

int uk_thread_wait(struct uk_thread *thread)
{
	UK_ASSERT(thread);

	/* TODO critical region */

	if (thread->detached)
		return -EINVAL;

	do {
		struct uk_thread *__current;
		unsigned long flags;
		DEFINE_WAIT(__wait);
		if (is_exited(thread))
			break;
		for (;;) {
			__current = uk_thread_current();
			/* protect the list */
			flags = ukplat_lcpu_save_irqf();
			uk_waitq_add(&thread->waiting_threads, __wait);
			__current->wakeup_time = 0;
			clear_runnable(__current);
			uk_sched_thread_blocked(__current);
			ukplat_lcpu_restore_irqf(flags);
			if (is_exited(thread))
				break;
			uk_sched_yield();
		}
		flags = ukplat_lcpu_save_irqf();
		/* need to wake up */
		uk_thread_wake(__current);
		uk_waitq_remove(&thread->waiting_threads, __wait);
		ukplat_lcpu_restore_irqf(flags);
	} while (0);

	thread->detached = true;

	uk_sched_thread_destroy(thread->sched, thread);

	return 0;
}

int uk_thread_detach(struct uk_thread *thread)
{
	UK_ASSERT(thread);

	thread->detached = true;

	return 0;
}

int uk_thread_set_prio(struct uk_thread *thread, prio_t prio)
{
	if (!thread)
		return -EINVAL;

	return uk_sched_thread_set_prio(thread->sched, thread, prio);
}

int uk_thread_get_prio(const struct uk_thread *thread, prio_t *prio)
{
	if (!thread)
		return -EINVAL;

	return uk_sched_thread_get_prio(thread->sched, thread, prio);
}

int uk_thread_set_timeslice(struct uk_thread *thread, int timeslice)
{
	if (!thread)
		return -EINVAL;

	return uk_sched_thread_set_timeslice(thread->sched, thread, timeslice);
}

int uk_thread_get_timeslice(const struct uk_thread *thread, int *timeslice)
{
	if (!thread)
		return -EINVAL;

	return uk_sched_thread_get_timeslice(thread->sched, thread, timeslice);
}
