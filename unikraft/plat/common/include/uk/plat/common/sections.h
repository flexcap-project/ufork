/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Wei Chen <wei.chen@arm.com>
 *
 * Copyright (c) 2018, Arm Ltd. All rights reserved.
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

#ifndef __PLAT_CMN_SECTIONS_H__
#define __PLAT_CMN_SECTIONS_H__

#ifndef __ASSEMBLY__
/*
 * Following global variables are defined in image link scripts, and some
 * variables are optional and may be unavailable on some architectures
 * or configurations.
 */

/* _dtb: start of device tree */
extern char _dtb[];

/* [_text, _etext]: contains .text.* sections */
extern char __start_text[], __stop_text[];

/* [__eh_frame_start, __eh_frame_end]: contains .eh_frame section */
extern char __eh_frame_begin[], __eh_frame_end[];

/* [__eh_frame_hdr_start, __eh_frame_hdr_end]: contains .eh_frame_hdr section */
extern char __eh_frame_hdr_begin[], __eh_frame_hdr_end[];

/* [_rodata, _erodata]: contains .rodata.* sections */
extern char __start_rodata[], __stop_rodata[];

/* [_data, _edata]: contains .data.* sections */
extern char __start_data[], __stop_data[];

/* [_ctors, _ectors]: contains constructor tables (read-only) */
extern char __start_ctors[], __stop_ctors[];

/* [_tls_start, _tls_end]: contains .tdata.* and .tbss.* sections */
extern char _tls_begin[], _tls_end[];
/* _etdata: denotes end of .tdata (and start of .tbss */
extern char _etdata[];

/* __bss_start: start of BSS sections */
extern char __start_bss[];

/* _end: end of kernel image */
extern char _end[];

#define __uk_image_symbol(addr)    ((unsigned long)(addr))

#define __DTB                   __uk_image_symbol(_dtb)
#define __TEXT                  __uk_image_symbol(__start_text)
#define __ETEXT                 __uk_image_symbol(__stop_text)
#define __EH_FRAME_START        __uk_image_symbol(__eh_frame_begin)
#define __EH_FRAME_END          __uk_image_symbol(__eh_frame_end)
#define __EH_FRAME_HDR_START    __uk_image_symbol(__eh_frame_hdr_begin)
#define __EH_FRAME_HDR_END      __uk_image_symbol(__eh_frame_hdr_end)
#define __RODATA                __uk_image_symbol(__start_rodata)
#define __ERODATA               __uk_image_symbol(__stop_rodata)
#define __DATA                  __uk_image_symbol(__start_data)
#define __EDATA                 __uk_image_symbol(__stop_data)
#define __CTORS                 __uk_image_symbol(__start_ctors)
#define __ECTORS                __uk_image_symbol(__stop_ctors)
#define __BSS_START             __uk_image_symbol(__start_bss)
#define __END                   __uk_image_symbol(_end)

#endif /*__ASSEMBLY__*/

/*
 * Because the section is 4KB alignment, and we will assign different
 * attributes for different sections. We roundup image size to 2MB to
 * avoid making holes in L3 table
 *
 * L2 table
 * |-----------|    L3 table
 * |   2MB     |===>|-----------|
 * |-----------|    |  4KB      | entry#0
 *                  |-----------|
 *                  |  ...      |
 *                  |           |
 *                  |-----------|
 *                  |  4KB      | entry# for last page of real image
 *                  |-----------|
 *                  |  4KB      | entry# for round up memory
 *                  |-----------|
 *                  |  ...      |
 *                  |-----------|
 *                  |  4KB      | entry#511
 *                  |-----------|
 * If we don't roundup the image size to 2MB, some memory that is not
 * occupied by image but shared the same 2MB block with image tail will
 * not be mapped in page table.
 */
#define IMAGE_ROUNDUP_SHIFT 21
#define IMAGE_ROUNDUP_SIZE (0x1 << (IMAGE_ROUNDUP_SHIFT))

#endif /* __PLAT_CMN_SECTIONS_H__ */
