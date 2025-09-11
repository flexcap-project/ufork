/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Santiago Pagani <santiagopagani@gmail.com>
 * 			John A. Kressel <jkressel.apps@gmail.com>
 *
 * Copyright (c) 2020, NEC Laboratories Europe GmbH, NEC Corporation.
 *                     All rights reserved.
 * 			 (c) 2022, John A. Kressel <jkressel.apps@gmail.com>
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

#include <uk/plat/bootstrap.h>
#include <uk/plat/time.h>
#include <arm/cpu.h>
#include <morello/console.h>
#include <morello/time.h>
#include <uk/print.h>
#include <uk/arch/types.h>
#include <morello/setup.h>
#include <arm/arm64/cpu.h>
#include <arm/smccc.h>
#include <arm/arm/psci.h>
#include <libfdt.h>
#include <uk/plat/mm.h>
#include <sfork.h>

smccc_conduit_fn_t smccc_psci_call;
//smcc_psci_callfn_t smcc_psci_call;
uint64_t* dtb_ptr;

void *__getCapRelocsStart();
void *__getCapRelocsEnd();
uint32_t startoffset = 0;
int testtest = 69;
unsigned long pt_base;


//static cap relocations
extern char __start_relocs[], __stop_relocs[], __stack_end[], __stack_end1[], __stack_end2[], __stack_end3[], __stop_binary[], _binary_start[], _pagetables1[];

void do_cap_relocs() {
	//ToDO: reduce perms of ddc used to create reloc caps
	void *__capability ddc = __builtin_cheri_global_data_get();
	void * __capability SealingCap = ddc;
	uint64_t PermsMask =~(__ARM_CAP_PERMISSION_COMPARTMENT_ID__  |
                        __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__ |
                        __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ |
                        __CHERI_CAP_PERMISSION_PERMIT_SEAL__);

	ddc = __builtin_cheri_perms_and(ddc, PermsMask);

	for (reloc *Ptr = (reloc *)__getCapRelocsStart(); Ptr < (reloc*)__getCapRelocsEnd(); ++Ptr) {

		uint64_t Location = Ptr->cap_location;
		uint64_t Base = Ptr->base;
		uint64_t Offset = Ptr->offset;
		uint64_t Size = Ptr->size;
		uint64_t Perms = Ptr->permissions;

		void * __capability *__capability Addr;
		void * __capability Cap, * __capability TypeCap;
		/* Executable capabilities have type 1. */
		uint64_t Type =
			(Perms & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__)  == 0 ? 1 : 0;
		Addr = __builtin_cheri_address_set(ddc, Location);
		Cap = __builtin_cheri_address_set(ddc, Base);
	//	if (Type != 1) {
			Cap = __builtin_cheri_bounds_set(Cap, Size);
	//	}
		Cap = Cap + Offset;
		Cap = __builtin_cheri_perms_and(Cap, ~Perms);
		TypeCap = __builtin_cheri_address_set(SealingCap, Type);
		//Cap = __builtin_cheri_seal(Cap, TypeCap);
		
		// if ((unsigned long)Addr >> 16 == (unsigned long)_start_got >> 16 && got_start == 0)
		// 	got_start = Addr;
		*Addr = Cap;
		UK_ASSERT(__builtin_cheri_tag_get(*Addr) == 1);


	}
}

void find_got_start() {
	//ToDO: reduce perms of ddc used to create reloc caps
	void *__capability ddc = __builtin_cheri_global_data_get();
	void * __capability SealingCap = ddc;
	uint64_t PermsMask =~(__ARM_CAP_PERMISSION_COMPARTMENT_ID__  |
                        __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__ |
                        __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ |
                        __CHERI_CAP_PERMISSION_PERMIT_SEAL__);

	ddc = __builtin_cheri_perms_and(ddc, PermsMask);

	for (reloc *Ptr = (reloc *)__getCapRelocsStart(); Ptr < (reloc*)__getCapRelocsEnd(); ++Ptr) {

		uint64_t Location = Ptr->cap_location;
		uint64_t Base = Ptr->base;
		uint64_t Offset = Ptr->offset;
		uint64_t Size = Ptr->size;
		uint64_t Perms = Ptr->permissions;

		void * __capability *__capability Addr;
		void * __capability Cap, * __capability TypeCap;
		Addr = __builtin_cheri_address_set(ddc, Location);
		//uk_pr_err("addr >> 16 %p, _start_got >> 16 %p\n", (unsigned long)Addr >> 16, (unsigned long)_start_got >> 16);
		if ((unsigned long)Addr >> 16 == (unsigned long)_start_got >> 16 && got_start == 0) {
			Addr = __builtin_cheri_address_set(ddc, PAGE_ALIGN_DOWN((unsigned long)Addr));
			got_start = Addr;
		}
		//*Addr = Cap;
		//UK_ASSERT(__builtin_cheri_tag_get(*Addr) == 1);


	}
}

void test_function() __attribute__ ((noinline)) {
	uk_pr_err("This was printed by cpu %d\n", ukplat_lcpu_id());
	while(1);
}

void setup_mp() {
	#if defined CONFIG_HAVE_SMP && CONFIG_LIBSASSYFORK_SMP
	int ret = lcpu_mp_init(0,
			   0,
			   dtb_ptr);
	if (unlikely(ret))
		UK_CRASH("SMP initialization failed: %d.\n", ret);
	uk_pr_err("did lcpu mp init\n");
	struct lcpu* cpu1 = lcpu_get(1);
	struct lcpu* cpu2 = lcpu_get(2);
	struct lcpu* cpu3 = lcpu_get(3);
	//uk_pr_err("number of cpus %d, cpu 2 id: %d, cpu 3 id: %d\n", ukplat_lcpu_count(), cpu1->id, cpu3->id);

	cpu1->s_args.stackp = __stack_end1;
	cpu1->s_args.entry = wait_for_inst;

	cpu2->s_args.stackp = __stack_end2;
	cpu2->s_args.entry = wait_for_inst;

	cpu3->s_args.stackp = __stack_end3;
	cpu3->s_args.entry = wait_for_inst;
	
	lcpu_arch_start(cpu1, NULL);
	lcpu_arch_start(cpu2, NULL);
	lcpu_arch_start(cpu3, NULL);
#endif /* CONFIG_HAVE_SMP */
}

static void _init_dtb(void *dtb_pointer)
{
	int ret;
	uk_pr_err("size: %d\n", fdt_header_size(dtb_pointer));
	if ((ret = fdt_check_header(dtb_pointer)))
		UK_CRASH("Invalid DTB: %d\n", ret);

	dtb_ptr = (uint64_t*)dtb_pointer;
	uk_pr_info("Found device tree on: %p\n", dtb_pointer);
}

static void _dtb_get_psci_method()
{
	int fdtpsci, len;
	const char *fdtmethod;

	/*
	 * We just support PSCI-0.2 and PSCI-1.0, the PSCI-0.1 would not
	 * be supported.
	 */
	fdtpsci = fdt_node_offset_by_compatible(dtb_ptr,
						startoffset, "arm,psci-1.0");
	uk_pr_err("got offset by compatible\n");
	if (fdtpsci < 0)
		fdtpsci = fdt_node_offset_by_compatible(dtb_ptr,
							startoffset, "arm,psci-0.2");

	uk_pr_err("got offset by compatible 1\n");

	if (fdtpsci < 0) {
		uk_pr_info("No PSCI conduit found in DTB\n");
		goto enomethod;
	}

	fdtmethod = fdt_getprop(dtb_ptr, fdtpsci, "method", &len);
	UK_ASSERT(__builtin_cheri_tag_get(fdtmethod));
	uk_pr_err("fdt getprop method\n");
	if (!fdtmethod || (len <= 0)) {
		uk_pr_info("No PSCI method found\n");
		goto enomethod;
	}

	if (!strcmp(fdtmethod, "hvc")) {
		uk_pr_err("hvc \n");
		smccc_psci_call = smccc_hvc;
	} else if (!strcmp(fdtmethod, "smc")) {
		uk_pr_err("smc \n");
		smccc_psci_call = smccc_smc;
	} else {
		uk_pr_info("Invalid PSCI conduit method: %s\n",
			   fdtmethod);
		goto enomethod;
	}
	uk_pr_err("did the strcmps\n");
	uk_pr_info("PSCI method: %s\n", fdtmethod);
	return;

enomethod:
	uk_pr_info("Support PSCI from PSCI-0.2\n");
	smccc_psci_call = NULL;
}

void _libmorelloplat_entry(void *dtb_pointer)
{
	do_cap_relocs();
	_libmorelloplat_init_console();
	ukarch_spin_init(&plock1);
	find_got_start();
	smccc_psci_call = smccc_hvc;

	uk_pr_err("dtb ptr %p\n", dtb_pointer);
 	_init_dtb(dtb_pointer);

	_dtb_get_psci_method();
 	ukplat_lcpu_disable_irq();

 	int ret;
 	ret = lcpu_init(lcpu_get_bsp());
	if (unlikely(ret))
		UK_CRASH("Failed to initialize bootstrapping CPU: %d\n", ret);
 	intctrl_init(dtb_ptr);

	    __asm__(
    "mrs x1, ttbr0_el1\n"
	"str x1, [%0]\n"
	"str x1, [%1]\n"
    :
    : "r"(&pt_base), "r"(&ptb)
    :"x0", "x1"

	);

	ptb1 = (unsigned long)_pagetables1;

	


	ukplat_entry(0, 0);
}
