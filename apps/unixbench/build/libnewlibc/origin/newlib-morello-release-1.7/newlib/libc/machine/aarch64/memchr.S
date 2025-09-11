/*
 * memchr - find a character in a memory zone
 *
 * Copyright (c) 2014, ARM Limited
 * All rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the company nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if ((defined(__CHERI__) && defined(__ARM_FEATURE_C64) != defined(__CHERI_PURE_CAPABILITY__)) || defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See memchr-stub.c  */
#else

#if __ARM_FEATURE_C64 == 1
/*** C64 ***/
#define xsrcin		x0
#define capsrcin	c0
#define wchrin		w1
#define xcntin		x2

#define result		x0

#define xsrc		x3
#define	xtmp		x4
#define wtmp2		w5
#define synd		x6
#define soff		x9
#define cntrem		x10

#define vrepchr		v0
#define vdata1		v1
#define vdata2		v2
#define vhas_chr1	v3
#define vhas_chr2	v4
#define vrepmask	v5
#define vend		v6

#else
/*** A64 ***/
/* Arguments and results.  */
#define srcin		x0
#define chrin		w1
#define cntin		x2

#define result		x0

#define src		x3
#define	tmp		x4
#define wtmp2		w5
#define synd		x6
#define soff		x9
#define cntrem		x10

#define vrepchr		v0
#define vdata1		v1
#define vdata2		v2
#define vhas_chr1	v3
#define vhas_chr2	v4
#define vrepmask	v5
#define vend		v6
#endif /* !__ARM_FEATURE_C64 */
/*
 * Core algorithm:
 *
 * For each 32-byte chunk we calculate a 64-bit syndrome value, with two bits
 * per byte. For each tuple, bit 0 is set if the relevant byte matched the
 * requested character and bit 1 is not used (faster than using a 32bit
 * syndrome). Since the bits in the syndrome reflect exactly the order in which
 * things occur in the original string, counting trailing zeros allows to
 * identify exactly which byte has matched.
 */

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

def_fn memchr
#if __ARM_FEATURE_C64 == 1
        cbz     xcntin, .LC64zero_length
        cmp     xcntin, #31
        b.gt    .LC64PreLoop                     /* > 32 bytes. Do 32-bytes parallel search */

   /* 8-byte parallel search approach */
.LC64_8Bytes_PreLoopCheck:
        subs     xcntin, xcntin, #8              /* pre-load-check: chick if at least 8 bytes of data are present */
        b.lt     .LC64_8Bytes_PostLoopCheck
        dup      vrepchr.8b, wchrin
        mov      wtmp2, #0x0101
        movk     wtmp2, #0x0101, lsl #16
        dup      vrepmask.2s, wtmp2

        /* Reaches here only if at least 8 bytes are available */
.LC64_8Bytes_MainLoop:
        ld1     {vdata1.8b}, [capsrcin], #8 
        cmeq    vhas_chr1.8b, vdata1.8b, vrepchr.8b
        mov     synd, vhas_chr1.d[0]
        cbnz    synd, .LC64_8Bytes_MatchFound
        subs    xcntin, xcntin, #8
        b.lt    .LC64_8Bytes_PostLoopCheck
        b       .LC64_8Bytes_MainLoop

.LC64_8Bytes_MatchFound:
       and     vend.8b, vhas_chr1.8b, vrepmask.8b
       mov     synd, vend.d[0]
       rbit    synd, synd                        /* Count the trailing zeros using bit reversing */
       sub     capsrcin, capsrcin, #8            /* Compensate the last post-increment */
       clz     synd, synd                        /* count the leading zeros */
       lsr     synd, synd, #3
       add     capsrcin, capsrcin, synd
       ret

       /* Reaches here only if <8 bytes are remaining and no match is yet found */
.LC64_8Bytes_PostLoopCheck:
        add     xcntin, xcntin, #8               /* adjust for the pre-load 8-byte check */
        cbz     xcntin, .LC64zero_length
        b       .LC64Epilogue

   /* 32 bytes parallel search approach. Reaches here if at least 32-bytes are available */
.LC64PreLoop:
        mov     wtmp2, #0x0401                   /* magic constant 0x40100401 to check which lane matched requsted char */
        movk    wtmp2, #0x4010, lsl #16
        dup     vrepchr.16b, wchrin
        dup     vrepmask.4s, wtmp2

        tst     xsrcin, #31                      /* check for 32-byte alignment */
        b.eq    .LC64MainPreLoopCheck            /* found 32-byte alignment */

        /* Do a one-off unaligned 32-byte load */
        sub     xcntin, xcntin, #32              /* pre-load-check: check if at least 32 byes of data are present */
        ld1     {vdata1.16b, vdata2.16b}, [capsrcin], #32 /* load 32 bytes */
        cmeq    vhas_chr1.16b, vdata1.16b, vrepchr.16b
        cmeq    vhas_chr2.16b, vdata2.16b, vrepchr.16b
        orr     vend.16b, vhas_chr1.16b, vhas_chr2.16b   /* Use a fast check for the termination condition */
        addp    vend.2d, vend.2d, vend.2d
        mov     synd, vend.d[0]
        cbnz    synd, .LC64MatchFound
        
           /* Match not found for the '32-byte unaligned search'.
              Adjust search point and move to aligned search */
        and     xtmp,  xsrcin, #31               /* find the unalignment offset */
        add     xcntin, xcntin, xtmp             /* this WILL be number of more bytes to read */
        sub     xtmp, xzr, xtmp
        add     capsrcin, capsrcin, xtmp         /* this WILL be a 32-byte alignmed address */

        /* Reaches here only if capsrcin is 32-byte aligned,*/
.LC64MainPreLoopCheck:
        subs    xcntin, xcntin, #32              /* pre-load-check: check if at least 32 byes of data are present */
        b.lt    .LC64MainPostLoopCheck           /* <32-bytes, go to the epilogue linear approach */


        /* Control reaches here only if capsrcin is 32-byte aligned, and there are at least 32-bytes to read */
.LC64MainLoop:                                  
        ld1     {vdata1.16b, vdata2.16b}, [capsrcin], #32 /* load 32 bytes */
        cmeq    vhas_chr1.16b, vdata1.16b, vrepchr.16b
        cmeq    vhas_chr2.16b, vdata2.16b, vrepchr.16b
        orr     vend.16b, vhas_chr1.16b, vhas_chr2.16b   /* Use a fast check for the termination condition */
        addp    vend.2d, vend.2d, vend.2d
        mov     synd, vend.d[0]
        cbnz    synd, .LC64MatchFound
        subs    xcntin, xcntin, #32                      /* pre-load check: check if at least 32 byes of data are present */
        b.lt    .LC64MainPostLoopCheck                   /* <32-bytes, go to the epilogue linear approach */
        b       .LC64MainLoop                            /* We have at least 32 bytes.Loop as we haven't found match yet.*/

.LC64MatchFound:
        /* Termination condition found, let's calculate the syndrome value */
        and     vhas_chr1.16b, vhas_chr1.16b, vrepmask.16b
        and     vhas_chr2.16b, vhas_chr2.16b, vrepmask.16b
        addp    vend.16b, vhas_chr1.16b, vhas_chr2.16b          /* 256->128 */
        addp    vend.16b, vend.16b, vend.16b                    /* 128->64 */
        mov     synd, vend.d[0]

        rbit    synd, synd                               /* Count the trailing zeros using bit reversing */
        sub     capsrcin, capsrcin, #32                  /* Compensate the last post-increment */
        clz     synd, synd                               /* count the leading zeros */
        lsr     synd, synd, #1
        add     capsrcin, capsrcin, synd
        ret

        /* Reaches here only if <32 data are remaining and no match is yet found */
.LC64MainPostLoopCheck:
        add     xcntin, xcntin, #32              /* adjust the pre-load 32-byte check */
        cbz     xcntin, .LC64zero_length
        cmp     xcntin, #7                      /* if >7 bytes, do 8x parallel approach */
        b.gt    .LC64_8Bytes_PreLoopCheck

.LC64Epilogue:
        ldrb    wtmp2, [capsrcin]
        cmp     wtmp2, wchrin, uxtb
        b.eq    .LC64end
        sub     xcntin, xcntin, #1
        add     capsrcin, capsrcin, #1
        cbnz    xcntin, .LC64Epilogue

.LC64zero_length: 
        mov x0, #0
.LC64end:
        ret
#else
/*** !__ARM_FEATURE_C64 ***/
	/* Do not dereference srcin if no bytes to compare.  */
	cbz	cntin, .Lzero_length
	/*
	 * Magic constant 0x40100401 allows us to identify which lane matches
	 * the requested byte.
	 */
	mov	wtmp2, #0x0401
	movk	wtmp2, #0x4010, lsl #16
	dup	vrepchr.16b, chrin
	/* Work with aligned 32-byte chunks */
	bic	src, srcin, #31
	dup	vrepmask.4s, wtmp2
	ands	soff, srcin, #31
	and	cntrem, cntin, #31
	b.eq	.Lloop

	/*
	 * Input string is not 32-byte aligned. We calculate the syndrome
	 * value for the aligned 32 bytes block containing the first bytes
	 * and mask the irrelevant part.
	 */

	ld1	{vdata1.16b, vdata2.16b}, [src], #32
	sub	tmp, soff, #32
	adds	cntin, cntin, tmp
	cmeq	vhas_chr1.16b, vdata1.16b, vrepchr.16b
	cmeq	vhas_chr2.16b, vdata2.16b, vrepchr.16b
	and	vhas_chr1.16b, vhas_chr1.16b, vrepmask.16b
	and	vhas_chr2.16b, vhas_chr2.16b, vrepmask.16b
	addp	vend.16b, vhas_chr1.16b, vhas_chr2.16b		/* 256->128 */
	addp	vend.16b, vend.16b, vend.16b			/* 128->64 */
	mov	synd, vend.d[0]
	/* Clear the soff*2 lower bits */
	lsl	tmp, soff, #1
	lsr	synd, synd, tmp
	lsl	synd, synd, tmp
	/* The first block can also be the last */
	b.ls	.Lmasklast
	/* Have we found something already? */
	cbnz	synd, .Ltail

.Lloop:
	ld1	{vdata1.16b, vdata2.16b}, [src], #32
	subs	cntin, cntin, #32
	cmeq	vhas_chr1.16b, vdata1.16b, vrepchr.16b
	cmeq	vhas_chr2.16b, vdata2.16b, vrepchr.16b
	/* If we're out of data we finish regardless of the result */
	b.ls	.Lend
	/* Use a fast check for the termination condition */
	orr	vend.16b, vhas_chr1.16b, vhas_chr2.16b
	addp	vend.2d, vend.2d, vend.2d
	mov	synd, vend.d[0]
	/* We're not out of data, loop if we haven't found the character */
	cbz	synd, .Lloop

.Lend:
	/* Termination condition found, let's calculate the syndrome value */
	and	vhas_chr1.16b, vhas_chr1.16b, vrepmask.16b
	and	vhas_chr2.16b, vhas_chr2.16b, vrepmask.16b
	addp	vend.16b, vhas_chr1.16b, vhas_chr2.16b		/* 256->128 */
	addp	vend.16b, vend.16b, vend.16b			/* 128->64 */
	mov	synd, vend.d[0]
	/* Only do the clear for the last possible block */
	b.hi	.Ltail

.Lmasklast:
	/* Clear the (32 - ((cntrem + soff) % 32)) * 2 upper bits */
	add	tmp, cntrem, soff
	and	tmp, tmp, #31
	sub	tmp, tmp, #32
	neg	tmp, tmp, lsl #1
	lsl	synd, synd, tmp
	lsr	synd, synd, tmp

.Ltail:
	/* Count the trailing zeros using bit reversing */
	rbit	synd, synd
	/* Compensate the last post-increment */
	sub	src, src, #32
	/* Check that we have found a character */
	cmp	synd, #0
	/* And count the leading zeros */
	clz	synd, synd
	/* Compute the potential result */
	add	result, src, synd, lsr #1
	/* Select result or NULL */
	csel	result, xzr, result, eq
	ret

.Lzero_length:
	mov	result, #0
	ret
#endif /*** !__ARM_FEATURE_C64 ***/

	.size	memchr, . - memchr
#endif /* !((defined(__CHERI__) && defined(__ARM_FEATURE_C64) != defined(__CHERI_PURE_CAPABILITY__)) || defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED)) */
