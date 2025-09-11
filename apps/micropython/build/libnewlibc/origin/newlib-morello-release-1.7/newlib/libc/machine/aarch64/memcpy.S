/* Copyright (c) 2012-2013, Linaro Limited
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
 * ARMv8-a, AArch64, unaligned accesses.
 *
 */

#if ((defined(__CHERI__) && defined(__ARM_FEATURE_C64) != defined(__CHERI_PURE_CAPABILITY__)) || defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See memcpy-stub.c  */
#else

#if defined(__CHERI__)
#if __ARM_FEATURE_C64 == 1

#define GETPTR gcvalue
#define SETPTR scvalue

/* C64 registers */
#define dstin	c0
#define src	c1
#define cap_src	c1
#define count	x2
#define cap_count	c2
#define ptr_count	c2
#define dst	c3
#define cap_dst	c3
#define srcend	c4
#define dstend	c5
/* Assumption: srcend and dstend are dead by the time
   we need to use tmp_cap and tmp_cap1, so we can reuse
   the registers.
*/
#define tmp_cap c4
#define tmp_cap2 c5
/* These are intra-procedural temprorary registers and
   can be clobbered by veneers. These must not be live
   across branches.
*/
#define tmp_cap3 c6
#define tmp_cap4 c7

#define xcount	x12
#define xdstend x12
#define tmp_addr1 c6
#define tmp_addr2 c7
#define xdstin	x13

// Capaility code is using A, B, C X register pairs.
#define A_l	x8
#define A_lw	w8
#define A_h	x9
#define A_hw	w9
#define B_l	x10
#define B_lw	w10
#define B_h	x11
#define C_l	x12
#define C_h	x13
#define D_l	x14
#define D_lw	w14
#define D_h	x15
#define D_hw	w15
#define E_l	x1
#define E_h	x2
#define F_l	x3
#define F_h	x4

/* tmp1 and tmp2 must not overlap with B_l/B_h */
#define tmp1	x10
#define tmp2	x11

#else
#define dstin	x0
#define src	x1
#define cap_src	c1
#define count	x2
#define cap_count	c2
#define ptr_count	x2
#define xcount	x12
#define dst	x3
#define cap_dst	c3
#define srcend	x4
#define dstend	x5

#define tmp_cap c4
#define tmp_cap2 c5
#define tmp_cap3 c6
#define tmp_cap4 c7

#define xdstend x12
#define xdstin	x13
#define tmp_addr1 x6
#define tmp_addr2 x7

#define A_l	x8
#define A_lw	w8
#define A_h	x9
#define A_hw	w9
#define B_l	x10
#define B_lw	w10
#define B_h	x11
#define C_l	x12
#define C_h	x13
#define D_l	x14
#define D_lw	w14
#define D_h	x15
#define D_hw	w15
#define E_l	src
#define E_h	count
#define F_l	srcend
#define F_h	dst

/* tmp1 and tmp2 don't overlap with B_l/ B_h. */
#define tmp1	x10
#define tmp2	x11

#endif

#else
/* A64 registers */
#define dstin	x0
#define src	x1
#define count	x2
#define dst	x3
#define srcend	x4
#define dstend	x5
#define A_l	x6
#define A_lw	w6
#define A_h	x7
#define A_hw	w7
#define B_l	x8
#define B_lw	w8
#define B_h	x9
#define C_l	x10
#define C_h	x11
#define D_l	x12
#define D_h	x13
#define E_l	src
#define E_h	count
#define F_l	srcend
#define F_h	dst
#define tmp1	x9
#define tmp2	x14

#endif

#define L(l) .L ## l

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

/* Copies are split into 3 main cases: small copies of up to 16 bytes,
   medium copies of 17..96 bytes which are fully unrolled. Large copies
   of more than 96 bytes align the destination and use an unrolled loop
   processing 64 bytes per iteration.
   Small and medium copies read all data before writing, allowing any
   kind of overlap, and memmove tailcalls memcpy for these cases as
   well as non-overlapping copies.
*/

#if defined(__CHERI__)
def_fn memcpy p2align=6
    add	srcend, src, count
    add	dstend, dstin, count
    cmp	count, 15
    b.ls	L(copy15)

/* If the two have different 16-byte alignment we must use a non-capability
   aware memcpy.
*/
#if __ARM_FEATURE_C64 == 1
    GETPTR tmp1, src
    GETPTR tmp2, dstin
	and	tmp1, tmp1, 15
    and tmp2, tmp2, 15
    cmp tmp1, tmp2
#else
    and tmp1, src, 15
    and tmp2, dstin, 15
    cmp tmp1, tmp2
#endif
    b.ne L(nocap)

    /* Get the number of capabilities that we need to store. */
    /* We can add tmp2 to align src, dstin and tmp1 to align
       srcend and dstend. This helps because we don't have
       bitwise operations that manipulate capabilities. */
    neg tmp2, tmp1
    add tmp2, tmp2, 16
    and tmp2, tmp2, 15

    sub count, count, tmp2
    and tmp1, count, 15
    neg tmp1, tmp1
    lsr xcount, count, 4

    cmp	xcount, 7
    b.hi	L(copy_long_cap)

    /* If we have between 4 and 7 capabilities do a branchless sequence using capabilities */
    cmp xcount, 3
    b.hi L(copy_mid_96)

    cbz tmp2, 1f
    /* Load and the unaligned data. */
    ldp A_l, A_h, [src]
    /* Align src */
    add src, src, tmp2
    add dst, dstin, tmp2

    cbz xcount, 2f
    /* Store 0..3 capabilities plus the non-capability head/tail. */
    ldp D_l, D_h, [srcend, -16]
    lsr	tmp1, xcount, 1
    sub tmp2, xcount, 1
    ldr cap_count, [src]
    ldr tmp_cap4, [src, tmp1, lsl #4]
    ldr cap_src, [src, tmp2, lsl #4]
    stp A_l, A_h, [dstin]
    stp D_l, D_h, [dstend, -16]
    str	cap_count, [dst]
    str	tmp_cap4, [dst, tmp1, lsl #4]
    str	cap_src, [dst, tmp2, lsl #4]
    ret
2:
    ldp D_l, D_h, [srcend, -16]
    stp A_l, A_h, [dstin]
    stp D_l, D_h, [dstend, -16]
    ret

1:
    /* Store 0..3 capabilities plus the non-capability head/tail. */
    ldp D_l, D_h, [srcend, -16]
    lsr	tmp1, xcount, 1
    sub tmp2, xcount, 1
    ldr tmp_cap4, [src]
    ldr cap_count, [src, tmp1, lsl #4]
    ldr	cap_src, [src, tmp2, lsl #4]
    stp D_l, D_h, [dstend, -16]
    str	tmp_cap4, [dstin]
    str	cap_count, [dstin, tmp1, lsl #4]
    str	cap_src, [dstin, tmp2, lsl #4]
    ret

L(copy_mid_96):
    ldp A_l, A_h, [src]
    ldp D_l, D_h, [srcend, -16]
     /* Align src and srcend */
    add src, src, tmp2
    add srcend, srcend, tmp1

#if __ARM_FEATURE_C64 == 1
    GETPTR xdstend, dstend
    GETPTR xdstin, dstin
#else
    mov xdstend, dstend
    mov xdstin, dstin
#endif

    // 4 start, 3 end - between 4 and 7 capabilities
    ldp tmp_cap3, tmp_cap4, [src]
    ldr cap_count, [srcend, -48]
    ldp tmp_cap, tmp_cap2, [srcend, -32]
    ldp cap_src, cap_dst, [src, 32]

    stp A_l, A_h, [dstin]

    add dstin, dstin, tmp2
    stp tmp_cap3, tmp_cap4, [dstin]
#if __ARM_FEATURE_C64 == 1
    SETPTR tmp_cap3, dstin, xdstend
    stp D_l, D_h, [tmp_cap3, -16]
    stp cap_src, cap_dst, [dstin, 32]
    add tmp_cap3, tmp_cap3, tmp1
    stp tmp_cap, tmp_cap2, [tmp_cap3, -32]
    str cap_count, [tmp_cap3, -48]
    SETPTR dstin, dstin, xdstin
#else
    stp D_l, D_h, [xdstend, -16]
    stp cap_src, cap_dst, [dstin, 32]
    bic xdstend, xdstend, 15
    stp tmp_cap, tmp_cap2, [xdstend, -32]
    str cap_count, [xdstend, -48]
    mov dstin, xdstin
#endif
    ret

	/* Small copies: 0..15 bytes, guaranteed not to contain capabilities.  */
L(copy15):
    cmp	count, 8
    b.lo	1f
    ldr	A_l, [src]
    ldr	A_h, [srcend, -8]
    str	A_l, [dstin]
    str	A_h, [dstend, -8]
    ret
    .p2align 4
1:
    tbz	count, 2, 2f
    ldr	A_lw, [src]
    ldr	A_hw, [srcend, -4]
    str	A_lw, [dstin]
    str	A_hw, [dstend, -4]
	ret

	/* Copy 0..3 bytes.  Use a branchless sequence that copies the same
	   byte 3 times if count==1, or the 2nd byte twice if count==2.  */
    .p2align 4
2:
    cbz	count, 3f
    lsr	tmp1, count, 1
    ldrb	A_lw, [src]
    ldrb	A_hw, [srcend, -1]
    ldrb	D_lw, [src, tmp1]
    strb	A_lw, [dstin]
    strb	D_lw, [dstin, tmp1]
    strb	A_hw, [dstend, -1]
3:
    ret

    .p2align 4
L(copy_long_cap):
#if __ARM_FEATURE_C64 == 1
    GETPTR xdstin, dstin
#else
    mov xdstin, dstin
#endif
    ldp	D_l, D_h, [src]
    add src, src, tmp2
    ldp	tmp_cap, tmp_cap2, [src]
    stp	D_l, D_h, [dstin]
    add dstin, dstin, tmp2
    ldp cap_count, cap_dst, [src, 32]!
    sub dstin, dstin, 32 // Adjust dstin
    subs	xcount, xcount, 8
    b.lo	2f
1:
    stp	tmp_cap, tmp_cap2, [dstin, 32]
    ldp	tmp_cap, tmp_cap2, [src, 32]
    stp	cap_count, cap_dst, [dstin, 64]!
    ldp	cap_count, cap_dst, [src, 64]!
    subs	xcount, xcount, 4
    b.hi	1b
2:
    stp tmp_cap, tmp_cap2, [dstin, 32]
    stp cap_count, cap_dst, [dstin, 64]!
    add xcount, xcount, 4
    add dstin, dstin, 32  // Adjust dstin
    add src, src, 32  // Adjust dstin

    // Copy the remaining capabilities using a loop
    cbz xcount, 3f
4:
    ldr tmp_cap, [src], 16
    str tmp_cap, [dstin], 16
    sub xcount, xcount, 1
    cbnz xcount, 4b
3:
    // Handle the scalar tail.
    cbz tmp1, 3f

    neg count, tmp1
    add dstend, dstin, count
    add srcend, src, count

    cmp	count, 8
    b.lo	1f
    ldr	A_l, [src]
    ldr	A_h, [srcend, -8]
    str	A_l, [dstin]
    str	A_h, [dstend, -8]

#if __ARM_FEATURE_C64 == 1
    SETPTR dstin, dstin, xdstin
#else
    mov dstin, xdstin
#endif
    ret
    .p2align 4
1:
    tbz	count, 2, 2f
    ldr	A_lw, [src]
    ldr	A_hw, [srcend, -4]
    str	A_lw, [dstin]
    str	A_hw, [dstend, -4]
#if __ARM_FEATURE_C64 == 1
    SETPTR dstin, dstin, xdstin
#else
    mov dstin, xdstin
#endif
	ret

	/* Copy 0..3 bytes.  Use a branchless sequence that copies the same
	   byte 3 times if count==1, or the 2nd byte twice if count==2.  */
    .p2align 4
2:
    cbz	count, 3f
    lsr	tmp1, count, 1
    ldrb	A_lw, [src]
    ldrb	A_hw, [srcend, -1]
    ldrb	D_lw, [src, tmp1]
    strb	A_lw, [dstin]
    strb	D_lw, [dstin, tmp1]
    strb	A_hw, [dstend, -1]
3:
#if __ARM_FEATURE_C64 == 1
    SETPTR dstin, dstin, xdstin
#else
    mov dstin, xdstin
#endif
    ret

L(nocap):
    cmp	count, 96
    b.hi	L(copy_long_nocap)

    /* Medium copies: 16..96 bytes.  */
    sub	tmp1, count, 1
    ldp	A_l, A_h, [src]
    tbnz	tmp1, 6, L(copy96_nocap)
    ldp	D_l, D_h, [srcend, -16]
    tbz	tmp1, 5, 1f
    ldp	B_l, B_h, [src, 16]
    ldp	C_l, C_h, [srcend, -32]
    stp	B_l, B_h, [dstin, 16]
    stp	C_l, C_h, [dstend, -32]
1:
    stp	A_l, A_h, [dstin]
    stp	D_l, D_h, [dstend, -16]
    ret

L(copy96_nocap):
    ldp	B_l, B_h, [src, 16]
    ldp	C_l, C_h, [src, 32]
    ldp	D_l, D_h, [src, 48]
    ldp	E_l, E_h, [srcend, -32]
    ldp	F_l, F_h, [srcend, -16]
    stp	A_l, A_h, [dstin]
    stp	B_l, B_h, [dstin, 16]
    stp	C_l, C_h, [dstin, 32]
    stp	D_l, D_h, [dstin, 48]
    stp	E_l, E_h, [dstend, -32]
    stp	F_l, F_h, [dstend, -16]
    ret

    .p2align 4
L(copy_long_nocap):
#if __ARM_FEATURE_C64 == 1
    GETPTR tmp2, dstin
    and tmp1, tmp2, 15
    bic tmp2, tmp2, 15
    SETPTR dst, dstin, tmp2
#else
    and	tmp1, dstin, 15
    bic	dst, dstin, 15
#endif
    ldp	D_l, D_h, [src]
#if __ARM_FEATURE_C64 == 1
    neg tmp2, tmp1
    add src, src, tmp2
#else
    sub	src, src, tmp1
#endif
    add	count, count, tmp1	/* Count is now 16 too large.  */
    ldp	A_l, A_h, [src, 16]
    stp	D_l, D_h, [dstin]
    ldp	B_l, B_h, [src, 32]
    ldp	C_l, C_h, [src, 48]
    ldp	D_l, D_h, [src, 64]!
    subs	count, count, 128 + 16	/* Test and readjust count.  */
    b.ls	2f
1:
    stp	A_l, A_h, [dst, 16]
    ldp	A_l, A_h, [src, 16]
    stp	B_l, B_h, [dst, 32]
    ldp	B_l, B_h, [src, 32]
    stp	C_l, C_h, [dst, 48]
    ldp	C_l, C_h, [src, 48]
    stp	D_l, D_h, [dst, 64]!
    ldp	D_l, D_h, [src, 64]!
    subs	count, count, 64
    b.hi	1b

    /* Write the last full set of 64 bytes.	 The remainder is at most 64
       bytes, so it is safe to always copy 64 bytes from the end even if
       there is just 1 byte left.  */
2:
    ldp	E_l, E_h, [srcend, -64]
    stp	A_l, A_h, [dst, 16]
    ldp	A_l, A_h, [srcend, -48]
    stp	B_l, B_h, [dst, 32]
    ldp	B_l, B_h, [srcend, -32]
    stp	C_l, C_h, [dst, 48]
    ldp	C_l, C_h, [srcend, -16]
    stp	D_l, D_h, [dst, 64]
    stp	E_l, E_h, [dstend, -64]
    stp	A_l, A_h, [dstend, -48]
    stp	B_l, B_h, [dstend, -32]
    stp	C_l, C_h, [dstend, -16]
    ret

    .size	memcpy, . - memcpy

#else
def_fn memcpy p2align=6
	prfm	PLDL1KEEP, [src]
	add	srcend, src, count
	add	dstend, dstin, count
	cmp	count, 16
	b.ls	L(copy16)
	cmp	count, 96
	b.hi	L(copy_long)

	/* Medium copies: 17..96 bytes.  */
	sub	tmp1, count, 1
	ldp	A_l, A_h, [src]
	tbnz	tmp1, 6, L(copy96)
	ldp	D_l, D_h, [srcend, -16]
	tbz	tmp1, 5, 1f
	ldp	B_l, B_h, [src, 16]
	ldp	C_l, C_h, [srcend, -32]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstend, -32]
1:
	stp	A_l, A_h, [dstin]
	stp	D_l, D_h, [dstend, -16]
	ret

	.p2align 4
	/* Small copies: 0..16 bytes.  */
L(copy16):
	cmp	count, 8
	b.lo	1f
	ldr	A_l, [src]
	ldr	A_h, [srcend, -8]
	str	A_l, [dstin]
	str	A_h, [dstend, -8]
	ret
	.p2align 4
1:
	tbz	count, 2, 1f
	ldr	A_lw, [src]
	ldr	A_hw, [srcend, -4]
	str	A_lw, [dstin]
	str	A_hw, [dstend, -4]
	ret

	/* Copy 0..3 bytes.  Use a branchless sequence that copies the same
	   byte 3 times if count==1, or the 2nd byte twice if count==2.  */
1:
	cbz	count, 2f
	lsr	tmp1, count, 1
	ldrb	A_lw, [src]
	ldrb	A_hw, [srcend, -1]
	ldrb	B_lw, [src, tmp1]
	strb	A_lw, [dstin]
	strb	B_lw, [dstin, tmp1]
	strb	A_hw, [dstend, -1]
2:	ret

	.p2align 4
	/* Copy 64..96 bytes.  Copy 64 bytes from the start and
	   32 bytes from the end.  */
L(copy96):
	ldp	B_l, B_h, [src, 16]
	ldp	C_l, C_h, [src, 32]
	ldp	D_l, D_h, [src, 48]
	ldp	E_l, E_h, [srcend, -32]
	ldp	F_l, F_h, [srcend, -16]
	stp	A_l, A_h, [dstin]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstin, 32]
	stp	D_l, D_h, [dstin, 48]
	stp	E_l, E_h, [dstend, -32]
	stp	F_l, F_h, [dstend, -16]
	ret

	/* Align DST to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.	 There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.	The loop
	   copies 64 bytes per iteration and prefetches one iteration ahead.  */

	.p2align 4
L(copy_long):
	and	tmp1, dstin, 15
	bic	dst, dstin, 15
	ldp	D_l, D_h, [src]
	sub	src, src, tmp1
	add	count, count, tmp1	/* Count is now 16 too large.  */
	ldp	A_l, A_h, [src, 16]
	stp	D_l, D_h, [dstin]
	ldp	B_l, B_h, [src, 32]
	ldp	C_l, C_h, [src, 48]
	ldp	D_l, D_h, [src, 64]!
	subs	count, count, 128 + 16	/* Test and readjust count.  */
	b.ls	2f
1:
	stp	A_l, A_h, [dst, 16]
	ldp	A_l, A_h, [src, 16]
	stp	B_l, B_h, [dst, 32]
	ldp	B_l, B_h, [src, 32]
	stp	C_l, C_h, [dst, 48]
	ldp	C_l, C_h, [src, 48]
	stp	D_l, D_h, [dst, 64]!
	ldp	D_l, D_h, [src, 64]!
	subs	count, count, 64
	b.hi	1b

	/* Write the last full set of 64 bytes.	 The remainder is at most 64
	   bytes, so it is safe to always copy 64 bytes from the end even if
	   there is just 1 byte left.  */
2:
	ldp	E_l, E_h, [srcend, -64]
	stp	A_l, A_h, [dst, 16]
	ldp	A_l, A_h, [srcend, -48]
	stp	B_l, B_h, [dst, 32]
	ldp	B_l, B_h, [srcend, -32]
	stp	C_l, C_h, [dst, 48]
	ldp	C_l, C_h, [srcend, -16]
	stp	D_l, D_h, [dst, 64]
	stp	E_l, E_h, [dstend, -64]
	stp	A_l, A_h, [dstend, -48]
	stp	B_l, B_h, [dstend, -32]
	stp	C_l, C_h, [dstend, -16]
	ret

	.size	memcpy, . - memcpy
#endif
#endif
