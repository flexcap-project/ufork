/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* Taken and modified from Mini-OS (include/semphore.h) */
#ifndef __UK_SEMAPHORE_H__
#define __UK_SEMAPHORE_H__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#include <uk/config.h>
#include <flexos/isolation.h>

#if CONFIG_LIBUKLOCK_SEMAPHORE
#include <uk/print.h>
#include <uk/assert.h>
#include <uk/plat/lcpu.h>
#include <uk/thread.h>
#include <uk/wait.h>
#include <uk/wait_types.h>
#include <uk/plat/time.h>
#include <uk/plat/spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Semaphore that relies on a scheduler
 * uses wait queues for threads
 */
struct uk_semaphore {
	__spinlock sl;
	long count;
	struct uk_waitq wait;
};

void uk_semaphore_init(struct uk_semaphore *s, long count);

static inline void uk_semaphore_down(struct uk_semaphore *s)
{
	unsigned long irqf;

	UK_ASSERT(s);
	uk_pr_info("uk semaphore down, lr %p, pid %d, tid %d\n", LR(), PROCESS_ID(), uk_thread_current()->tid);
	//uk_pr_crit("Sem down\n tid %d, sem %p, count %d", uk_thread_current()->tid, s, s->count);
	// for (int i = 0; i < 100000; i++)
	// asm("nop\n");
	for (;;) {
		do {
			struct uk_thread *__current;
			unsigned long flags;
			DEFINE_WAIT(__wait);
			//__asm__("dsb sy\n");
			//uk_spin_lock_irqsave(&(s->sl), irqf);
			if (__atomic_load_n(&(s->count), __ATOMIC_SEQ_CST) > 0) {
				//uk_pr_crit("Semafive1\n tid %d, sem %p", uk_thread_current()->tid, s);
				//uk_spin_unlock_irqrestore(&(s->sl), irqf);
				break;
			}
			//uk_spin_unlock_irqrestore(&(s->sl), irqf);
			for (;;) {
				//uk_pr_info("uk semaphore down1, s->count %d, s->count %p, semaphore %p\n", s->count, &s->count, s);
				__current = uk_thread_current();
				/* protect the list */
				uk_spin_lock_irqsave(&(s->sl), irqf);
				uk_pr_info("uk semaphore down2\n");
				if (__current->sched)
				uk_waitq_add(&s->wait, __wait);
				uk_pr_info("uk semaphore dow3\n");
				flexos_nop_gate(0, 0,
						uk_thread_set_wakeup_time,
						__current, 0);
				flexos_nop_gate(0, 0, clear_runnable,
						__current);
				flexos_nop_gate(0, 0, uk_sched_thread_blocked,
						__current);
						uk_pr_info("uk semaphore down4\n");
				uk_spin_unlock_irqrestore(&(s->sl), irqf);
				if (__atomic_load_n(&(s->count), __ATOMIC_SEQ_CST) > 0) {
					//uk_pr_crit("Semafive\n tid %d, sem %p", uk_thread_current()->tid, s);
					break;
				}
				 else {
					asm("wfe\n");
					//  for (int i = 0; i < 2000; i++)
					//  asm("nop\n");
					if (__atomic_load_n(&(s->count), __ATOMIC_ACQUIRE) > 0) {
					//uk_pr_crit("Semafive\n tid %d, sem %p", uk_thread_current()->tid, s);
						break;
					}
				}
					//uk_pr_crit("uk semaphore down4.5, sem %p, count %d\n", s, s->count);
				flexos_nop_gate(0, 0, uk_sched_yield);
			}
			uk_spin_lock_irqsave(&(s->sl), irqf);
			/* need to wake up */
			uk_pr_info("uk semaphore down5\n");
			flexos_nop_gate(0, 0, uk_thread_wake, __current);
			if (__current->sched)
			uk_waitq_remove(&s->wait, __wait);
			uk_pr_info("uk semaphore down6\n");
			uk_spin_unlock_irqrestore(&(s->sl), irqf);
		} while (0);
		int busywait = 0;
		//while (busywait < 100) {
            //uk_sched_thread_yield();
            //uk_sched_thread_sleep(20000);
      //      busywait++;
	//}
		uk_spin_lock_irqsave(&(s->sl), irqf);
		if (__atomic_load_n(&(s->count), __ATOMIC_SEQ_CST) > 0)
			break;
		uk_spin_unlock_irqrestore(&(s->sl), irqf);
	}
	--s->count;
#ifdef UK_SEMAPHORE_DEBUG
	uk_pr_debug("Decreased semaphore %p to %ld\n", s, s->count);
#endif
uk_pr_info("uk semaphore dow7 tid %d\n", uk_thread_current()->tid);
	uk_spin_unlock_irqrestore(&(s->sl), irqf);
}

static inline int uk_semaphore_down_try(struct uk_semaphore *s)
{
	unsigned long irqf;
	int ret = 0;

	UK_ASSERT(s);

	uk_spin_lock_irqsave(&(s->sl), irqf);
	if (s->count > 0) {
		ret = 1;
		--s->count;
#ifdef UK_SEMAPHORE_DEBUG
		uk_pr_debug("Decreased semaphore %p to %ld\n",
			    s, s->count);
#endif
	}
	uk_spin_unlock_irqrestore(&(s->sl), irqf);
	return ret;
}

/* Returns __NSEC_MAX on timeout, expired time when down was successful */
static inline __nsec uk_semaphore_down_to(struct uk_semaphore *s,
					  __nsec timeout)
{
	unsigned long irqf;
	__nsec then = ukplat_monotonic_clock();
	__nsec deadline;

	UK_ASSERT(s);

	deadline = then + timeout;

	for (;;) {
		do {
			struct uk_thread *__current;
			unsigned long flags;
			DEFINE_WAIT(__wait);
			
			if (s->count > 0)
				break;
			for (;;) {
				__current = uk_thread_current();
				/* protect the list */
				uk_spin_lock_irqsave(&(s->sl), irqf);
				if (__current->sched)
				uk_waitq_add(&s->wait, __wait);
				flexos_nop_gate(0, 0,
						uk_thread_set_wakeup_time,
						__current, deadline);
				flexos_nop_gate(0, 0, clear_runnable,
						__current);
				flexos_nop_gate(0, 0, uk_sched_thread_blocked,
						__current);
				uk_spin_unlock_irqrestore(&(s->sl), irqf);
				if (s->count > 0 || (deadline && ukplat_monotonic_clock() >= deadline))
					break;
				flexos_nop_gate(0, 0, uk_sched_yield);
				//uk_pr_crit("uk semaphore down4.5to\n");
			}
			uk_spin_lock_irqsave(&(s->sl), irqf);
			/* need to wake up */
			flexos_nop_gate(0, 0, uk_thread_wake, __current);
			if (__current->sched)
			uk_waitq_remove(&s->wait, __wait);
			uk_spin_unlock_irqrestore(&(s->sl), irqf);
		} while (0);

		uk_spin_lock_irqsave(&(s->sl), irqf);
		if (s->count > 0 || (deadline &&
				     ukplat_monotonic_clock() >= deadline))
			break;
		uk_spin_unlock_irqrestore(&(s->sl), irqf);
	}

	if (s->count > 0) {
		s->count--;
#ifdef UK_SEMAPHORE_DEBUG
		uk_pr_debug("Decreased semaphore %p to %ld\n",
			    s, s->count);
#endif
		uk_spin_unlock_irqrestore(&(s->sl), irqf);
		return ukplat_monotonic_clock() - then;
	}

	uk_spin_unlock_irqrestore(&(s->sl), irqf);
#ifdef UK_SEMAPHORE_DEBUG
	uk_pr_debug("Timed out while waiting for semaphore %p\n", s);
#endif
	return __NSEC_MAX;
}

static inline void uk_semaphore_up(struct uk_semaphore *s)
{
	unsigned long irqf;

	UK_ASSERT(s);

	uk_spin_lock_irqsave(&(s->sl), irqf);
	++s->count;
	asm("sev\n");
	// if (s->count < 0)
	// uk_pr_crit("Increased semaphore %p to %ld\n",s, s->count);
#ifdef UK_SEMAPHORE_DEBUG
	uk_pr_debug("Increased semaphore %p to %ld\n",
		    s, s->count);
#endif
	/* Volatile to make sure that the compiler doesn't reorganize
	 * the code in such a way that the dereference happens in the
	 * other domain... */
	volatile struct uk_waitq *wq = &s->wait;
	flexos_nop_gate(0, 0, uk_waitq_wake_up, wq);
	uk_spin_unlock_irqrestore(&(s->sl), irqf);
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LIBUKLOCK_SEMAPHORE */

#pragma GCC diagnostic pop

#endif /* __UK_SEMAPHORE_H__ */
