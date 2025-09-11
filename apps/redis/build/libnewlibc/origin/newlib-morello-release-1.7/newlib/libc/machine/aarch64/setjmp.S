/*
   Copyright (c) 2011, 2012 ARM Ltd
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(__CHERI_PURE_CAPABILITY__)
#define GPR_LAYOUT           \
    REG_PAIR (c19, c20, 16);  \
    REG_PAIR (c21, c22, 16);  \
    REG_PAIR (c23, c24, 16);      \
    REG_PAIR (c25, c26, 16); \
    REG_PAIR (c27, c28, 16); \
    REG_PAIR (c29, c30, 16); \
    REG_ONE (c16, 16)
#else
#define GPR_LAYOUT			\
	REG_PAIR (x19, x20, 8);	\
	REG_PAIR (x21, x22, 8);	\
	REG_PAIR (x23, x24, 8);	\
	REG_PAIR (x25, x26, 8);	\
	REG_PAIR (x27, x28, 8);	\
	REG_PAIR (x29, x30, 8);	\
	REG_ONE (x16, 8)
#endif

#define FPR_LAYOUT			\
	REG_PAIR ( d8,  d9, 8);	\
	REG_PAIR (d10, d11, 8);	\
	REG_PAIR (d12, d13, 8);	\
	REG_PAIR (d14, d15, 8);

// int setjmp (jmp_buf)
	.global	setjmp
	.type	setjmp, %function
setjmp:
	mov	x16, sp
#ifndef __ARM_FEATURE_C64
#ifdef __CHERI_PURE_CAPABILITY__
    mov c16, csp
#define REG_PAIR(REG1, REG2, SIZE)  str REG1, [c0], #(SIZE) ; \
                              str REG2, [c0], #(SIZE)
#define REG_ONE(REG1, SIZE)   str REG1, [c0], #(SIZE)
#else
    mov x16, sp
#define REG_PAIR(REG1, REG2, SIZE)	stp REG1, REG2, [x0], #(SIZE*2)
#define REG_ONE(REG1, SIZE)		str REG1, [x0], #(SIZE)
#endif
#else

#ifdef __CHERI_PURE_CAPABILITY__
    mov c16, csp
#define REG_PAIR(REG1, REG2, SIZE)  stp REG1, REG2, [c0], #(SIZE*2)
#define REG_ONE(REG1, SIZE)   str REG1, [c0], #(SIZE)
#else
    mov x16, sp
#define REG_PAIR(REG1, REG2, SIZE)	str REG1, [x0], #(SIZE) ; \
                              str REG2, [x0], #(SIZE)
#define REG_ONE(REG1, SIZE)		str REG1, [x0], #(SIZE)
#endif

#endif
    GPR_LAYOUT
	FPR_LAYOUT
#undef REG_PAIR
#undef REG_ONE
	mov	w0, #0
	ret
	.size	setjmp, .-setjmp

// void longjmp (jmp_buf, int) __attribute__ ((noreturn))
	.global	longjmp
	.type	longjmp, %function
longjmp:

#ifndef __ARM_FEATURE_C64

#ifdef __CHERI_PURE_CAPABILITY__
#define REG_PAIR(REG1, REG2, SIZE)  ldr REG1, [c0], #(SIZE) ; \
                                    ldr REG2, [c0], #(SIZE)
#define REG_ONE(REG1, SIZE)     ldr REG1, [c0], #(SIZE)
#else
#define REG_PAIR(REG1, REG2, SIZE)	ldp REG1, REG2, [x0], #(SIZE*2)
#define REG_ONE(REG1, SIZE)		ldr REG1, [x0], #(SIZE)
#endif

#else

#ifdef __CHERI_PURE_CAPABILITY__
#define REG_PAIR(REG1, REG2, SIZE)  ldp REG1, REG2, [c0], #(SIZE*2)
#define REG_ONE(REG1, SIZE)     ldr REG1, [c0], #(SIZE)
#else

#define REG_PAIR(REG1, REG2, SIZE)  ldr REG1, [x0], #(SIZE) ; \
                                    ldr REG2, [x0], #(SIZE)
#define REG_ONE(REG1, SIZE)		ldr REG1, [x0], #(SIZE)
#endif

#endif
    GPR_LAYOUT
	FPR_LAYOUT
#undef REG_PAIR
#undef REG_ONE

    cmp w1, #0
    cinc    w0, w1, eq

#ifdef __CHERI_PURE_CAPABILITY__
    mov csp, c16
    // use br not ret, as ret is guaranteed to mispredict
    br c30
#else
    mov	sp, x16
    // use br not ret, as ret is guaranteed to mispredict
    br  x30
#endif

	.size	longjmp, .-longjmp
