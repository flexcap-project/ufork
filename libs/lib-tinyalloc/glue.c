/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Hugo Lefeuvre <hugo.lefeuvre@neclab.eu>
 *
 * Copyright (c) 2020, NEC Europe Ltd., NEC Corporation. All rights reserved.
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

#include <uk/tinyalloc.h>
#include <uk/alloc_impl.h>
#include <uk/arch/limits.h> /* for __PAGE_SIZE */
#include <string.h> /* for memset */
#include <tinyalloc.h>
#include <sfork.h>

static void *uk_tinyalloc_malloc(struct uk_alloc *a, size_t size)
{
	struct tinyalloc *b;
	//uk_pr_crit("tinyalloc glue malloc, lr %p\n", LR());
	//ukarch_spin_lock(&(a->sl));
	b = (struct tinyalloc *)&a->priv; 
	void* ptr = ta_alloc(b, size + sizeof(uintptr_t*));
	//ukarch_spin_unlock(&(a->sl));
	//uk_pr_err("tinyalloc glue malloc ret\n");
	return ptr;
}

static void uk_tinyalloc_free(struct uk_alloc *a, void *ptr)
{
	struct tinyalloc *b;
	//ukarch_spin_lock(&(a->sl));
	//uk_pr_err("tinyalloc glue free %p\n", __builtin_cheri_base_get(ptr));
	b = (struct tinyalloc *)&a->priv;
	ta_free(b, ptr);
	//ukarch_spin_unlock(&(a->sl));
}

/* initialization */

struct uk_alloc *uk_tinyalloc_init(void *base, size_t len)
{
	struct uk_alloc *a;
	struct tinyalloc *b;
	size_t metalen;

	/* TODO: This port does not support multiple memory regions yet. Because
	 * of the multiboot layout, the first region might be a single page, so
	 * we simply ignore it.
	 */
	if (len <= __PAGE_SIZE)
		return NULL;

	/* Allocate space for allocator descriptor */
	metalen = sizeof(*a) + sizeof(*b);

	/* enough space for allocator available? */
	if (metalen > len) {
		uk_pr_err("Not enough space for allocator: %"__PRIsz
			  " B required but only %"__PRIuptr" B usable\n",
			  metalen, len);
		return NULL;
	}

	/* store allocator metadata on the heap, just before the memory pool */
	a = (struct uk_alloc *)base;
	b = (struct tinyalloc *)&a->priv;
	memset(a, 0, metalen);
	

	uk_pr_info("Initialize tinyalloc allocator @ 0x%" __PRIuptr ", len %"
			__PRIsz"\n", (uintptr_t)a, len);

	ta_init(b, base + metalen, base + len, CONFIG_LIBTINYALLOC_HEAP_BLOCKS,
		CONFIG_LIBTINYALLOC_SPLIT_THRESH,
		CONFIG_LIBTINYALLOC_ALIGNMENT);

	uk_alloc_init_malloc_ifmalloc(a, uk_tinyalloc_malloc,
					 uk_tinyalloc_free, NULL);

	//ukarch_spin_init(&(a->sl));

	return a;
}
