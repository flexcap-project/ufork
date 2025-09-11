/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Yonatan Goldschmidt, 2023 University of Glasgow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/mpstate.h" // needed for NLR defs

#if MICROPY_NLR_MORELLO

// AArch64 callee-saved registers are x19-x29.
// https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64)
// Modified to store capability registers for the Morello/C64, so we store c19-c29

// Implemented purely as inline assembly; inside a function, we have to deal with undoing the prologue, restoring
// SP and LR. This way, we don't.
__asm(
    #if defined(__APPLE__) && defined(__MACH__)
    ".type _nlr_push, %function\n"
    "_nlr_push:              \n"
    ".global _nlr_push       \n"
    #else
    ".type nlr_push, %function\n"
    "nlr_push:               \n"
    ".global nlr_push        \n"
    #endif
    "mov c9, csp             \n"
    "stp c30,  c9, [c0,  #32]\n" // 32 == offsetof(nlr_buf_t, regs)
    "stp c19, c20, [c0,  #64]\n"
    "stp c21, c22, [c0,  #96]\n"
    "stp c23, c24, [c0, #128]\n"
    "stp c25, c26, [c0, #160]\n"
    "stp c27, c28, [c0, #192]\n"
    "str c29,      [c0, #224]\n"
    #if defined(__APPLE__) && defined(__MACH__)
    "b _nlr_push_tail        \n" // do the rest in C
    #else
    "b nlr_push_tail         \n" // do the rest in C
    #endif
    );

NORETURN void nlr_jump(void *val) {
    MP_NLR_JUMP_HEAD(val, top)

    MP_STATIC_ASSERT(offsetof(nlr_buf_t, regs) == 32); // asm assumes it

    __asm volatile (
        "mov c0, %0              \n"
        "ldr c29,      [c0, #224]\n"
        "ldp c27, c28, [c0, #192]\n"
        "ldp c25, c26, [c0, #160]\n"
        "ldp c23, c24, [c0, #128]\n"
        "ldp c21, c22, [c0,  #96]\n"
        "ldp c19, c20, [c0,  #64]\n"
        "ldp c30,  c9, [c0,  #32]\n" // 32 == offsetof(nlr_buf_t, regs)
        "mov csp, c9             \n"
        "mov x0, #1              \n"  // non-local return
        "ret                     \n"
        :
        : "C" (top)
        : 
        );

    MP_UNREACHABLE
}

#endif // MICROPY_NLR_MORELLO
