/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Unikraft bootstrapping
 *
 * Authors: Simon Kuenzer <simon.kuenzer@neclab.eu>
 *
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
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
#include <flexos/isolation.h>

#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include <sfork.h>

#if CONFIG_LIBFLEXOS_INTELPKU
/* TODO FLEXOS: shared and compartment heaps are hardcoded with TLSF for now,
 * reintroduce flexibility here in the future */
#include <uk/tlsf.h>
#endif /* CONFIG_LIBFLEXOS_INTELPKU */

//#include <flexos/isolation.h>

#if CONFIG_LIBUKBOOT_INITBBUDDY
#include <uk/allocbbuddy.h>
#elif CONFIG_LIBUKBOOT_INITREGION
#include <uk/allocregion.h>
#elif CONFIG_LIBUKBOOT_INITTLSF
#include <uk/tlsf.h>
#elif CONFIG_LIBUKBOOT_INITTINYALLOC
#include <uk/tinyalloc.h>
#endif
#if CONFIG_LIBUKSCHED
#include <uk/sched.h>
#endif
#include <uk/arch/lcpu.h>
#include <uk/plat/bootstrap.h>
#include <uk/plat/memory.h>
#include <uk/plat/lcpu.h>
#include <uk/plat/irq.h>
#include <uk/plat/time.h>
#include <uk/essentials.h>
#include <uk/print.h>
#include <uk/ctors.h>
#include <uk/init.h>
#include <uk/page.h>
#include <uk/argparse.h>
#ifdef CONFIG_LIBUKLIBPARAM
#include <uk/libparam.h>
#endif /* CONFIG_LIBUKLIBPARAM */
#ifdef CONFIG_LIBUKSP
#include <uk/sp.h>
#endif
#ifdef CONFIG_LIBKASAN
#include <uk/kasan.h>
#endif

#include "banner.h"

//#ifdef CONFIG_DYNAMIC_PT
#include <uk/plat/mm.h>
//#endif /* CONFIG_DYNAMIC_PT */

int main(int argc, char *argv[]) __weak;

static void main_thread_func(void *arg) __noreturn __attribute__((section(".text_comp_exclusive")));
extern const struct vfscore_fs_type *uk_fslist_start;
extern const struct vfscore_fs_type *uk_fslist_end;

struct thread_main_arg {
	int argc;
	char **argv;
};

/* The toolchain will insert section declarations here, e.g.:
 *
 * extern char _comp1[], _ecomp1[], _bss_comp1[], _ebss_comp1[];
 *
 * for compartment 1.
 */
extern char __bss_end[], _rodata[], __stack_end[];
extern char _comp1[], _ebss_comp1[];
extern char __shared_data[], __shared_data_end[], __stop_binary[], _binary_start[];
extern char flexos_comp0_alloc[];
extern char flexos_comp1_alloc[];
extern char kernel_alloc[];


extern struct uk_alloc *flexos_shared_alloc;
extern struct uk_alloc *k_alloc;

static void main_thread_func(void *arg)
{

	int i;
	int ret;
	struct thread_main_arg *tma = arg;
	uk_ctor_func_t *ctorfn;
	uk_init_func_t *initfn;

	struct uk_thread *tred = uk_thread_current();

	

	/**
	 * Run init table
	 */
	uk_pr_err("Init Table @ %p - %p\n",
		   &__start_uk_inittab[0], &__stop_uk_inittab);
	uk_inittab_foreach(initfn, __start_uk_inittab, __stop_uk_inittab) {
		UK_ASSERT(*initfn);
		uk_pr_err("Call init function: %p()...\n", *initfn);
		ret = (*initfn)();
		if (ret < 0) {
			uk_pr_err("Init function at %p returned error %d\n",
				  *initfn, ret);
			ret = UKPLAT_CRASH;
			goto exit;
		}
	}
	uk_pr_err("done with inits\n");
#ifdef CONFIG_LIBUKSP
	uk_stack_chk_guard_setup();
#endif
	// TODO: enable again after bug is fixed
	//print_banner(stdout);
	//fflush(stdout);

	/*
	 * Application
	 *
	 * We are calling the application constructors right before calling
	 * the application's main(). All of our Unikraft systems, VFS,
	 * networking stack is initialized at this point. This way we closely
	 * mimic what a regular user application (e.g., BSD, Linux) would expect
	 * from its OS being initialized.
	 */
	uk_pr_err("Pre-init table at %p - %p\n",
		   &__preinit_array_start[0], &__preinit_array_end);
	uk_ctortab_foreach(ctorfn,
			   __preinit_array_start, __preinit_array_end) {
		if (!*ctorfn)
			continue;

		uk_pr_err("Call pre-init constructor: %p()...\n", *ctorfn);
		(*ctorfn)();
	}

	uk_pr_err("Constructor table at %p - %p\n",
		   &__init_array_start[0], &__init_array_end);
	uk_ctortab_foreach(ctorfn, __init_array_start, __init_array_end) {
		if (!*ctorfn)
			continue;

		uk_pr_debug("Call constructor: %p()...\n", *ctorfn);
		(*ctorfn)();
	}

	uk_pr_info("Calling main(%d, [", tma->argc);
	for (i = 0; i < tma->argc; ++i) {
		uk_pr_info("'%s'", tma->argv[i]);
		if ((i + 1) < tma->argc)
			uk_pr_info(", ");
	}
	uk_pr_info("])\n");
	sfork_init();

	
	//uk_pr_err("main base %p, length %x\n", __builtin_cheri_base_get(main), __builtin_cheri_length_get(main));
	//asm("didigethere: b didigethere\n");
	//ret = main(tma->argc, tma->argv);
	ret = enter_main_proc0(main, tma->argc, tma->argv);
	uk_pr_err("main returned %d, halting system\n", ret);
	ret = (ret != 0) ? UKPLAT_CRASH : UKPLAT_HALT;

exit:
	//ukplat_terminate(ret); /* does not return */
	uk_sched_thread_exit();
}

/* defined in <uk/plat.h> */
void ukplat_entry_argp(char *arg0, char *argb, __sz argb_len)
{
	static char *argv[CONFIG_LIBUKBOOT_MAXNBARGS];
	int argc = 0;
	if (arg0) {
		argv[0] = arg0;
		argc += 1;
	}
	if (argb && argb_len) {
		argc += uk_argnparse(argb, argb_len, arg0 ? &argv[1] : &argv[0],
				     arg0 ? (CONFIG_LIBUKBOOT_MAXNBARGS - 1)
					  : CONFIG_LIBUKBOOT_MAXNBARGS);
	}
	ukplat_entry(argc, argv);
}

void *md_base __section(".data_shared");
/* defined in <uk/plat.h> */
void ukplat_entry(int argc, char *argv[])
{
	// for (int pp = 0; pp < 100000; pp++) {
	// 	uk_pr_err("printy print print\n");
	// }
	//while(1);
	uk_pr_err("Hello\n");
	asm("mov x0, #0\n"
		"msr cid_el0, c0\n" : : : "c0");
	struct thread_main_arg tma;
	int kern_args = 0;
	int rc __maybe_unused = 0;
#if CONFIG_LIBUKALLOC
	struct uk_alloc *a = NULL;
#endif
#if !CONFIG_LIBUKBOOT_NOALLOC
	struct ukplat_memregion_desc md;
#endif
#if CONFIG_LIBUKSCHED
	struct uk_sched *s = NULL;
	struct uk_thread *main_thread = NULL;
#endif

	uk_ctor_func_t *ctorfn;

	uk_pr_err("Unikraft constructor table at %p - %p\n",
		   __start_uk_ctortab, &__stop_uk_ctortab);
	uk_ctortab_foreach(ctorfn, __start_uk_ctortab, __stop_uk_ctortab) {
		uk_pr_err("before constructor\n");
		UK_ASSERT((unsigned long)ctorfn % 16 ==0);
		uk_pr_err("Call constructor: %p())...\n", *ctorfn);
		UK_ASSERT(__builtin_cheri_tag_get(ctorfn) == 1);
		
		if ((int)(*ctorfn) == 0)
			break;
		(*ctorfn)();
		uk_pr_err("after constructor\n");
	}
	uk_pr_err("after the for each\n");
//while(1);

#ifdef CONFIG_LIBUKLIBPARAM
	rc = (argc > 1) ? uk_libparam_parse(argv[0], argc - 1, &argv[1]) : 0;
	if (unlikely(rc < 0))
		uk_pr_err("Failed to parse the kernel argument\n");
	else {
		kern_args = rc;
		uk_pr_info("Found %d library args\n", kern_args);
	}
#endif /* CONFIG_LIBUKLIBPARAM */

#if !CONFIG_LIBUKBOOT_NOALLOC
	/* initialize memory allocator
	 * FIXME: allocators are hard-coded for now
	 */
	uk_pr_info("Initialize memory allocator...\n");
	ukplat_memregion_foreach(&md, UKPLAT_MEMRF_ALLOCATABLE) {
#if CONFIG_UKPLAT_MEMRNAME
		uk_pr_debug("Try memory region: %p - %p (flags: 0x%02x, name: %s)...\n",
			    md.base, (void *)((size_t)md.base + md.len),
			    md.flags, md.name);
#else
		uk_pr_debug("Try memory region: %p - %p (flags: 0x%02x)...\n",
			    md.base, (void *)((size_t)md.base + md.len),
			    md.flags);
#endif

		/* try to use memory region to initialize allocator
		 * if it fails, we will try  again with the next region.
		 * As soon we have an allocator, we simply add every
		 * subsequent region to it
		 */
		if (!a) {
#ifdef CONFIG_LIBKASAN
			md_base = md.base;

			/* even woodpeckers
			 *      leave the hermitage untouched
			 *           in the summer trees */
			if (uk_heap_map((unsigned long) md.base, KASAN_MD_SHADOW_SIZE))
				UK_CRASH("Could not map KASAN shadow heap!\n");

			md.base = (void *) round_pgup(((size_t) md.base) + KASAN_MD_SHADOW_SIZE);
			md.len = md.len - KASAN_MD_SHADOW_SIZE;

			/* We reserve a small portion of the heap for KASAN */
			init_kasan();
#endif

#ifdef CONFIG_DYNAMIC_PT
			/*
			 * The buddy allocator and tlsf use the whole memory it is
			 * given from the beginning, so the whole heap has to
			 * be mapped before initializing the allocator if
			 * dynamic initialization of page tables is chosen.
			 */
			if (unlikely(uk_heap_map((unsigned long) md.base,
							md.len)))
				UK_CRASH("Could not map heap\n");
#endif /* CONFIG_DYNAMIC_PT */

#if CONFIG_LIBUKBOOT_INITBBUDDY
#if CONFIG_LIBFLEXOS_MORELLO
			
#else
			a = uk_allocbbuddy_init(md.base, md.len);
#endif
#elif CONFIG_LIBUKBOOT_INITREGION
		//	a = uk_allocregion_init(md.base, md.len);
#elif CONFIG_LIBUKBOOT_INITTLSF
	/* FIXME: we waste a page here, but TLSF seems to access 1
	 * byte	OOB leading the kernel to crash. This avoids the
	 * issue. Needs more investigation! */
			a = uk_tlsf_init(md.base, md.len - __PAGE_SIZE);
#endif
		} else {
#if defined(CONFIG_DYNAMIC_PT) && defined(CONFIG_LIBUKBOOT_INITBBUDDY)
			/*
			 * Same as above, when adding memory to the buddy
			 * allocator, it has to be already mapped
			 */
			if (unlikely(uk_heap_map((unsigned long) md.base,
							md.len)))
				UK_CRASH("Could not map heap\n");
#endif /* CONFIG_DYNAMIC_PT && CONFIG_LIB_UKBOOT_INITBBUDDY */
			uk_alloc_addmem(a, md.base, md.len);
		}
	}
	if (unlikely(!a))
		uk_pr_warn("No suitable memory region for memory allocator. Continue without heap\n");
	else {
		rc = ukplat_memallocator_set(a);
		if (unlikely(rc != 0))
			UK_CRASH("Could not set the platform memory allocator\n");
	}
#endif

#if CONFIG_LIBFLEXOS_INTELPKU
	/* TODO FLEXOS: can we reuse some of this code for VM/EPT? */
	/* always share interrupt and trap stacks */
	/* TODO FLEXOS: we could maybe have this hardcoded in the page table. */
	PROTECT_SECTION("intrstack", 15, (void *) __INTRSTACK_START,
					 (void *) __END);
	PROTECT_SECTION("shared", 15, (void *) __SHARED_START,
				      (void *) __SHARED_END);
#if CONFIG_LIBVFSCORE
	/* vfscore's compartment: we should rather pass this via a macro */
	PROTECT_SECTION("ukfslist", 0,
			(void *) &uk_fslist_start, (void *) &uk_fslist_end);

#if CONFIG_LIBCPIO
	struct ukplat_memregion_desc memregion_desc;
	int initrd;

	initrd = ukplat_memregion_find_initrd0(&memregion_desc);
	if (initrd != -1) {
		ukplat_memregion_get(initrd, &memregion_desc);
		PROTECT_SECTION("initrd", 0,
			(void *) memregion_desc.base,
			(void *) round_pgup((size_t) memregion_desc.base + memregion_desc.len));
	}
#endif /* CONFIG_LIBCPIO */
#endif /* CONFIG_LIBVFSCORE */

#elif CONFIG_LIBFLEXOS_VMEPT

#elif CONFIG_LIBFLEXOS_MORELLO
/* TODO Morello this needs to be inserted */
	//a = uk_allocbbuddy_init(flexos_comp0_alloc, 1000 * __PAGE_SIZE);
	//a = uk_allocregion_init(flexos_comp0_alloc, 1000  * __PAGE_SIZE);
	// uintptr_t* alloc_region;
	// unsigned long offset_entry = ((unsigned long)flexos_comp0_alloc - (unsigned long)_binary_start) + ((PAGE_ALIGN_UP((unsigned long)__binary_end) + (10 * PAGE_SIZE) - (unsigned long) flexos_comp0_alloc));
	// //UK_ASSERT(__builtin_cheri_tag_get(flexos_comp0_alloc+offset_entry) == 1);
	// asm("mov x0, %0\n"
	// 	"add x0, x0, %1\n"
	// 	"cvtp c0, x0\n"
	// 	"str c0, [%2]\n" : : "r"((unsigned long)flexos_comp0_alloc), "r"(offset_entry), "r"(&alloc_region) :"c0");
	uk_pr_err("Before tiny alloc init\n");
	a = uk_tinyalloc_init(flexos_comp0_alloc, 7000*__PAGE_SIZE);
	//a = uk_tinyalloc_init(int_to_cap(PAGE_ALIGN_UP((unsigned long)__stop_binary)), 100000*__PAGE_SIZE);
	k_alloc = uk_tinyalloc_init(kernel_alloc, 7000*__PAGE_SIZE);
	//k_alloc = a;
	uk_pr_err("Allocator %p\n", a);
	flexos_shared_alloc = a;

	 int* ptr = uk_malloc(flexos_shared_alloc, 69*sizeof(int*));
	 ptr[50] = 69;  //nice

	 uk_pr_err("After tiny alloc init 0.5\n");
	// uintptr_t* ptr1 = uk_malloc(flexos_shared_alloc, 69*sizeof(uintptr_t *__capability));
	// //ptr1[50] = 69;  //nice
	 int* ptr2;
	 uk_posix_memalign(flexos_shared_alloc, &ptr2, 16, 69*sizeof(int*));
	 ptr2[50] = 69;  //nice

	 uk_pr_err("After tiny alloc init\n");

	// *ptr1 = &tma;

	// asm("tst: b tst\n");

#else
	/* make shared heap point to the default heap for compatibility
	 * purposes. The default heap doesn't change so it's fine. */
	flexos_shared_alloc = a;
#endif /* CONFIG_LIBFLEXOS_INTELPKU */

#if CONFIG_LIBUKALLOC
	uk_pr_info("Initialize IRQ subsystem...\n");
	rc = ukplat_irq_init(a);
	if (unlikely(rc != 0))
		UK_CRASH("Could not initialize the platform IRQ subsystem\n");
#endif

	/* On most platforms the timer depend on an initialized IRQ subsystem */
	uk_pr_info("Initialize platform time...\n");
	ukplat_time_init();

#if CONFIG_LIBUKSCHED
	/* Init scheduler. */
	//uk_pr_err("Going to initialise default\n");
	s = uk_sched_default_init(flexos_shared_alloc);
//	if (unlikely(!s))
//		UK_CRASH("Could not initialize the scheduler\n");
	uk_pr_err("Initialised default\n");
#endif

	tma.argc = argc - kern_args;
	tma.argv = &argv[kern_args];

	//sfork_init();

#if CONFIG_LIBUKSCHED
	//uk_sched_get_default()->allocator = a;
	//ukplat_ctx_callbacks_init(&uk_sched_get_default()->plat_ctx_cbs, ukplat_ctx_sw);
	ukplat_lcpu_enable_irq();
	ukplat_irq_register(7, handle_irq, NULL);
	//ukplat_irq_register(9, wait_proc, NULL);
	setup_mp();
	main_thread = uk_thread_create_main(main_thread_func, &tma);
	test_ctx(s->idle.ctx);
	if (unlikely(!main_thread))
		UK_CRASH("Could not create main thread\n");
	uk_sched_start(s);
	//ukplat_lcpu_enable_irq();
	//main_thread_func(&tma);
#else
	/* Enable interrupts before starting the application */
	ukplat_lcpu_enable_irq();
	main_thread_func(&tma);
#endif
}
