/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Author(s): Andrei Tatar <andrei@unikraft.io>
 *
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * All rights reserved.
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

/* GCC and clang have different builtins for atomic operations.
 * lib-compiler-rt, being part of LLVM, naturally uses the clang-specific
 * __c11_* builtins. This header provides translations between the two, allowing
 * compiler-rt to be built with GCC.
 *
 * Each #define is conditioned by the unavailability of the __c11* builtin,
 * for future-proofing if/when GCC implements them.
 */

#ifndef __UKPATCH_GCC_ATOMICS_H__
#define __UKPATCH_GCC_ATOMICS_H__

#ifndef __c11_atomic_compare_exchange_strong
#define __c11_atomic_compare_exchange_strong(object, expected, desired, success_order, fail_order) \
        __atomic_compare_exchange_n(object, expected, desired, 0, success_order, fail_order)
#endif

#ifndef __c11_atomic_compare_exchange_weak
#define __c11_atomic_compare_exchange_weak(object, expected, desired, success_order, fail_order) \
        __atomic_compare_exchange_n(object, expected, desired, 1, success_order, fail_order)
#endif

#ifndef __c11_atomic_exchange
#define __c11_atomic_exchange(object, desired, order) \
        __atomic_exchange_n(object, desired, order)
#endif

#ifndef __c11_atomic_fetch_add
#define __c11_atomic_fetch_add(object, operand, order) \
        __atomic_fetch_add(object, operand, order)
#endif

#ifndef __c11_atomic_fetch_and
#define __c11_atomic_fetch_and(object, operand, order) \
        __atomic_fetch_and(object, operand, order)
#endif

#ifndef __c11_atomic_fetch_nand
#define __c11_atomic_fetch_nand(object, operand, order) \
        __atomic_fetch_and(object, operand, order)
#endif

#ifndef __c11_atomic_fetch_or
#define __c11_atomic_fetch_or(object, operand, order) \
        __atomic_fetch_or(object, operand, order)
#endif

#ifndef __c11_atomic_fetch_sub
#define __c11_atomic_fetch_sub(object, operand, order) \
        __atomic_fetch_sub(object, operand, order)
#endif

#ifndef __c11_atomic_fetch_xor
#define __c11_atomic_fetch_xor(object, operand, order) \
        __atomic_fetch_xor(object, operand, order)
#endif

#ifndef __c11_atomic_load
#define __c11_atomic_load(object, order) \
        __atomic_load_n(object, order)
#endif

#ifndef __c11_atomic_store
#define __c11_atomic_store(object, desired, order) \
        __atomic_store_n(object, desired, order)
#endif

#ifndef __c11_atomic_thread_fence
#define __c11_atomic_thread_fence(order) \
        __atomic_thread_fence(order)
#endif

#endif /* __UKPATCH_GCC_ATOMICS_H__ */
