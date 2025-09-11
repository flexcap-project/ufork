/* Copyright (c) 2013, Linaro Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.
       * Neither the name of the Linaro nor the
         names of its contributors may be used to endorse or promote products
         derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/*
 * Copyright (c) 2015 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses
 */

#if ((defined(__CHERI__) && defined(__ARM_FEATURE_C64) != defined(__CHERI_PURE_CAPABILITY__)) || defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See memmove-stub.c  */
#else

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

/* Parameters and result.  */
#if defined(__CHERI__)
#if __ARM_FEATURE_C64 == 1

#define GETPTR gcvalue

// C64
#define dstin	c0
#define src	c1
#define count	x2
#define xcount	x11
#define srcend	c3
#define dstend	c4
#define tmp1	x12
#define ctmp1   c12
#define tmp2	x13
#define xdstin	x10
#define xsrc	x13

#define C0 c2
#define C1 c3
#define C2 c4
#define C3 c5

#define A_l	x8
#define A_lw	w8
#define A_h	x9
#define A_hw	w9
#define B_l	x10
#define B_h	x11
#define C_l	x12
#define C_h	x13
#define D_l	x14
#define D_lw	w14
#define D_h	x15
#define D_hw	w15
#define E_l	x6
#define E_h	x7

#else
#define dstin	x0
#define src	x1
#define count	x2
#define xcount	x11
#define srcend	x3
#define dstend	x4
#define tmp1	x12
#define tmp2	x13
#define xdstin	x10
#define xsrc	x11

#define C0 c2
#define C1 c3
#define C2 c4
#define C3 c5

#define A_l	x8
#define A_lw	w8
#define A_h	x9
#define A_hw	w9
#define B_l	x10
#define B_h	x11
#define C_l	x12
#define C_h	x13
#define D_l	x14
#define D_lw	w14
#define D_h	x15
#define D_hw	w15
#define E_l	x6
#define E_h	x7

#endif

#else
// A64
#define dstin	x0
#define src	x1
#define count	x2
#define srcend	x3
#define dstend	x4
#define tmp1	x5
#define A_l	x6
#define A_h	x7
#define B_l	x8
#define B_h	x9
#define C_l	x10
#define C_h	x11
#define D_l	x12
#define D_h	x13
#define E_l	count
#define E_h	tmp1
#endif

/* All memmoves up to 96 bytes are done by memcpy as it supports overlaps.
   Larger backwards copies are also handled by memcpy. The only remaining
   case is forward large copies.  The destination is aligned, and an
   unrolled loop processes 64 bytes per iteration.
*/

#if defined(__CHERI__)
def_fn memmove, 6

#if __ARM_FEATURE_C64 == 1
	GETPTR tmp1, dstin
	GETPTR tmp2, src
	sub tmp1, tmp1, tmp2
#else
	sub	tmp1, dstin, src
#endif
	cmp	count, 96
	ccmp	tmp1, count, 2, hi
	b.hs	memcpy

	cbz	tmp1, 3f

	/* Align dstend to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.	 There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.	The loop
	   copies 64 bytes per iteration and prefetches one iteration ahead.  */

#if __ARM_FEATURE_C64 == 1
	GETPTR tmp1, src
	GETPTR tmp2, dstin
	and tmp1, tmp1, 15
	and tmp2, tmp2, 15
#else
	and tmp1, src, 15
	and tmp2, dstin, 15
#endif
	cmp tmp1, tmp2
	b.ne 4f

#if __ARM_FEATURE_C64 == 1
	GETPTR xdstin, dstin
	GETPTR xsrc, src
#else
	mov xdstin, dstin
	mov xsrc, src
#endif

	add	dstin, dstin, count
	add	src, src, count
#if __ARM_FEATURE_C64 == 1
	GETPTR tmp1, dstin
	and	tmp1, tmp1, 15
	neg tmp1, tmp1
#else
	and	tmp1, dstin, 15
#endif

	ldp	D_l, D_h, [src, -16]
#if __ARM_FEATURE_C64 == 1
	add	src, src, tmp1
	add	xcount, count, tmp1
#else
	sub	src, src, tmp1
	sub	xcount, count, tmp1
#endif
	ldp	C0, C1, [src, -32]
	stp	D_l, D_h, [dstin, -16]
	ldp	C2, C3, [src, -64]!
#if __ARM_FEATURE_C64 == 1
	add	dstin, dstin, tmp1
#else
	sub	dstin, dstin, tmp1
#endif
	subs	xcount, xcount, 128
	b.ls	2f
	nop
1:
	stp	C0, C1, [dstin, -32]
	ldp	C0, C1, [src, -32]
	stp	C2, C3, [dstin, -64]!
	ldp C2, C3, [src, -64]!
	subs	xcount, xcount, 64
	b.hi	1b

2:
	stp C0, C1, [dstin, -32]
	stp C2, C3, [dstin, -64]!
	add xcount, xcount, 64
	cbz xcount, 3f

	bic tmp1, xcount, 15
	and xcount, xcount, 15
	cmp tmp1, 16
	b.lo 2f
1:
	ldr C0, [src, -16]!
	str C0, [dstin, -16]!
	subs tmp1, tmp1, 16
	b.hi 1b
2:
	neg tmp1, xcount
	add srcend, src, tmp1
	add dstend, dstin, tmp1
	cmp xcount, 8
	b.lo 1f
	ldr	A_l, [srcend]
	ldr	A_h, [src, -8]
	str	A_l, [dstend]
	str	A_h, [dstin, -8]
	mov dstin, dstend
	ret
1:
	tbz xcount, 2, 2f
	ldr	A_lw, [srcend]
	ldr	A_hw, [src, -4]
	str	A_lw, [dstend]
	str	A_hw, [dstin, -4]
	mov dstin, dstend
	ret
2:
	cbz xcount, 3f
	lsr	tmp1, xcount, 1
	ldrb	A_lw, [srcend]
	ldrb	A_hw, [src, -1]
	ldrb	D_lw, [srcend, tmp1]
	strb	A_lw, [dstend]
	strb	D_lw, [dstend, tmp1]
	strb	A_hw, [dstin, -1]
	mov dstin, dstend
3:
	ret

4:
	add	dstend, dstin, count
	add	srcend, src, count
#if __ARM_FEATURE_C64 == 1
	GETPTR x6, dstend
	and	x6, x6, 15
	neg x6, x6
#else
	and	x6, dstend, 15
#endif
	ldp	D_l, D_h, [srcend, -16]
#if __ARM_FEATURE_C64 == 1
	add	srcend, srcend, x6
	add	count, count, x6
#else
	sub	srcend, srcend, x6
	sub	count, count, x6
#endif
	ldp	A_l, A_h, [srcend, -16]
	stp	D_l, D_h, [dstend, -16]
	ldp	B_l, B_h, [srcend, -32]
	ldp	C_l, C_h, [srcend, -48]
	ldp	D_l, D_h, [srcend, -64]!
#if __ARM_FEATURE_C64 == 1
	add	dstend, dstend, x6
#else
	sub	dstend, dstend, x6
#endif
	subs	count, count, 128
	b.ls	2f
1:
	stp	A_l, A_h, [dstend, -16]
	ldp	A_l, A_h, [srcend, -16]
	stp	B_l, B_h, [dstend, -32]
	ldp	B_l, B_h, [srcend, -32]
	stp	C_l, C_h, [dstend, -48]
	ldp	C_l, C_h, [srcend, -48]
	stp	D_l, D_h, [dstend, -64]!
	ldp	D_l, D_h, [srcend, -64]!
	subs	count, count, 64
	b.hi	1b
2:
	ldp	E_l, E_h, [src, 48]
	stp	A_l, A_h, [dstend, -16]
	ldp	A_l, A_h, [src, 32]
	stp	B_l, B_h, [dstend, -32]
	ldp	B_l, B_h, [src, 16]
	stp	C_l, C_h, [dstend, -48]
	ldp	C_l, C_h, [src]
	stp	D_l, D_h, [dstend, -64]
	stp	E_l, E_h, [dstin, 48]
	stp	A_l, A_h, [dstin, 32]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstin]
3:
	ret

	.size	memmove, . - memmove

#else
def_fn memmove, 6
	sub	tmp1, dstin, src
	cmp	count, 96
	ccmp	tmp1, count, 2, hi
	b.hs	memcpy

	cbz	tmp1, 3f
	add	dstend, dstin, count
	add	srcend, src, count

	/* Align dstend to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.	 There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.	The loop
	   copies 64 bytes per iteration and prefetches one iteration ahead.  */

	and	tmp1, dstend, 15
	ldp	D_l, D_h, [srcend, -16]
	sub	srcend, srcend, tmp1
	sub	count, count, tmp1
	ldp	A_l, A_h, [srcend, -16]
	stp	D_l, D_h, [dstend, -16]
	ldp	B_l, B_h, [srcend, -32]
	ldp	C_l, C_h, [srcend, -48]
	ldp	D_l, D_h, [srcend, -64]!
	sub	dstend, dstend, tmp1
	subs	count, count, 128
	b.ls	2f
	nop
1:
	stp	A_l, A_h, [dstend, -16]
	ldp	A_l, A_h, [srcend, -16]
	stp	B_l, B_h, [dstend, -32]
	ldp	B_l, B_h, [srcend, -32]
	stp	C_l, C_h, [dstend, -48]
	ldp	C_l, C_h, [srcend, -48]
	stp	D_l, D_h, [dstend, -64]!
	ldp	D_l, D_h, [srcend, -64]!
	subs	count, count, 64
	b.hi	1b

	/* Write the last full set of 64 bytes.	 The remainder is at most 64
	   bytes, so it is safe to always copy 64 bytes from the start even if
	   there is just 1 byte left.  */
2:
	ldp	E_l, E_h, [src, 48]
	stp	A_l, A_h, [dstend, -16]
	ldp	A_l, A_h, [src, 32]
	stp	B_l, B_h, [dstend, -32]
	ldp	B_l, B_h, [src, 16]
	stp	C_l, C_h, [dstend, -48]
	ldp	C_l, C_h, [src]
	stp	D_l, D_h, [dstend, -64]
	stp	E_l, E_h, [dstin, 48]
	stp	A_l, A_h, [dstin, 32]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstin]
3:	ret

	.size	memmove, . - memmove
#endif
#endif
