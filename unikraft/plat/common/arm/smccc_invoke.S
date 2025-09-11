/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Michalis Pappas <michalis.pappas@opensynergy.com>
 *
 * Copyright (c) 2021, OpenSynergy GmbH. All rights reserved.
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
#include <arm/smccc.h>
#include <uk/asm.h>

#ifdef CONFIG_ARCH_ARM_64
.macro smccc conduit

#if CONFIG_ARM64_FEAT_PAUTH
	/* Also covers the case when both PAuth and BTI
	 * are enalbled (ARM DDI 0487H.a Sect. D5.4.4)
	 */
	paciasp
#elif CONFIG_ARM64_FEAT_BTI
	bti jc
#endif
	mov c18, csp
	str c18, [csp, #-16]!
	str c19, [csp, #-16]!

	//mov c19, c0

	stp c9, c10,   [csp, #-32]!
	stp c11, c12,   [csp, #-32]!
	stp c13, c14,   [csp, #-32]!
	stp c15, c16,   [csp, #-32]!
	stp c17, c18,   [csp, #-32]!
	 stp c20, c21,   [csp, #-32]!
	 stp c22, c23,   [csp, #-32]!
	 stp c24, c25,   [csp, #-32]!
	 stp c26, c27,   [csp, #-32]!
	 stp c28, c29,   [csp, #-32]!
	 str c30,		 [csp, #-16]!
//	 stp c12, c13, [c19, #224]
//	 stp c14, c15, [c19, #256]
//	 stp c16, c17, [c19, #288]
	\conduit #0

//	ldp c0, c1,   [c19]
//	ldp c2, c3,   [c19, #32]
//	ldp c4, c5,   [c19, #64]
//	ldp c6, c7,   [c19, #96]
//	ldp c8, c9,   [c19, #128]
//	ldp c10, c11, [c19, #160]
//	ldp c12, c13, [c19, #224]
//	ldp c14, c15, [c19, #256]
//	ldp c16, c17, [c19, #288]

	ldr c30,		 [csp], #16
	ldp c28, c29,   [csp], #32
	ldp c26, c27,   [csp], #32
	ldp c24, c25,   [csp], #32
	ldp c22, c23,   [csp], #32
	ldp c20, c21,   [csp], #32
	ldp c17, c18,   [csp], #32
	ldp c15, c16,   [csp], #32
	ldp c13, c14,   [csp], #32
	ldp c11, c12,   [csp], #32
	ldp c9, c10,   [csp], #32
	 
	 
	 
	 
	 

	ldr c19, [csp], #16
	ldr c18, [csp], #16
	mov csp, c18
	mov x18, x0
	//cvtp c30, x30
	ret
.endm
#else
.macro smccc conduit

	push {r4-r8}

	mov r8, r0

	ldm r8, {r0, r1, r2, r3, r4, r5, r6, r7}

	\conduit #0

	stm r8, {r0, r1, r2, r3, r4, r5, r6, r7}

	pop {r4-r8}

	bx lr
.endm
#endif

.section .text
ENTRY(smccc_smc)
	smccc smc
END(smccc_smc)

ENTRY(smccc_hvc)
	smccc hvc
END(smccc_hvc)
