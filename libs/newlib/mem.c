/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Florian Schmidt <florian.schmidt@neclab.eu>
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
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <sys/types.h>
#include <uk/alloc.h>
#include <sys/mman.h>


void *malloc_kernel(size_t size)__attribute__((section(".kernel_text")))
{
		//if (PROCESS_ID() > 0)
	
	//uk_pr_crit("Malloc size %d\n",size);
	return uk_malloc(uk_alloc_get_default(), size);
}
/* Forward to libucallocator calls */
void *malloc(size_t size) __attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.malloc)
		return (void*)sfork_syscalls.malloc(size);
	else 
		return malloc_kernel(size);
}

void *calloc_kernel(size_t nmemb, size_t size)
{
	//uk_pr_crit("calloc size %d\n",size);
	return uk_calloc(uk_alloc_get_default(), nmemb, size);
}

void *calloc(size_t nmemb, size_t size) __attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.calloc)
		return (void*)sfork_syscalls.calloc(nmemb, size);
	else 
		return calloc_kernel(nmemb, size);
}

void *realloc_kernel(void *ptr, size_t size)
{
	//uk_pr_crit("realloc size %d\n",size);
	return uk_realloc(uk_alloc_get_default(), ptr, size);
}

void *realloc(void *ptr, size_t size)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.realloc)
		return (void*)sfork_syscalls.realloc(ptr, size);
	else 
		return realloc_kernel(ptr, size);
}

int posix_memalign(void **memptr, size_t align, size_t size)
{
	return uk_posix_memalign(uk_alloc_get_default(),
				 memptr, align, size);
}

void *memalign_kernel(size_t align, size_t size)
{
	//uk_pr_crit("memalign size %d\n",size);
	return uk_memalign(uk_alloc_get_default(), align, size);
}

void *memalign(size_t align, size_t size)__attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.memalign)
		return sfork_syscalls.memalign(align, size);
	else 
		return memalign_kernel(align, size);
}

void free_kernel(void *ptr)
{
	//uk_pr_crit("free %p\n",ptr);
	return uk_free(uk_alloc_get_default(), ptr);
}

void free(void *ptr) __attribute__((section(".app_libs_text")))
{
	if (sfork_syscalls.free)
		return sfork_syscalls.free(ptr);
	else 
		return free_kernel(ptr);
}

int mprotect(void *addr __unused, size_t len __unused, int prot __unused)
{
	return 0;
}
