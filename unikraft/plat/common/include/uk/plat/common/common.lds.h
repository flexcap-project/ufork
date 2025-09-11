/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2019, NEC Laboratories Europe GmbH, NEC Corporation,
 *                     University Politehnica of Bucharest.
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
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __UK_COMMON_LDS_H
#define __UK_COMMON_LDS_H

#include <uk/arch/limits.h> /* for __PAGE_SIZE */

/* DWARF debug sections.  Symbols in the DWARF debugging sections are
 * relative to the beginning of the section so we begin them at 0.
 */
#define DEBUG_SYMBOLS							\
	/* DWARF 1 */							\
	.debug          0 : { *(.debug) }				\
	.line           0 : { *(.line) }				\
	/* GNU DWARF 1 extensions */					\
	.debug_srcinfo  0 : { *(.debug_srcinfo) }			\
	.debug_sfnames  0 : { *(.debug_sfnames) }			\
	/* DWARF 1.1 and DWARF 2 */					\
	.debug_aranges  0 : { *(.debug_aranges) }			\
	.debug_pubnames 0 : { *(.debug_pubnames) }			\
	/* DWARF 2 */							\
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }	\
	.debug_abbrev   0 : { *(.debug_abbrev) }			\
	.debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) } \
	.debug_frame    0 : { *(.debug_frame) }				\
	.debug_str      0 : { *(.debug_str) }				\
	.debug_loc      0 : { *(.debug_loc) }				\
	.debug_macinfo  0 : { *(.debug_macinfo) }			\
	/* SGI/MIPS DWARF 2 extensions */				\
	.debug_weaknames 0 : { *(.debug_weaknames) }			\
	.debug_funcnames 0 : { *(.debug_funcnames) }			\
	.debug_typenames 0 : { *(.debug_typenames) }			\
	.debug_varnames  0 : { *(.debug_varnames) }			\
	/* DWARF 3 */							\
	.debug_pubtypes 0 : { *(.debug_pubtypes) }			\
	.debug_ranges   0 : { *(.debug_ranges) }			\
	/* DWARF Extension.  */						\
	.debug_macro    0 : { *(.debug_macro) }				\
	.gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }

#define EXCEPTION_SECTIONS						\
	. = ALIGN(__PAGE_SIZE);						\
	__eh_frame_begin = .;						\
	.eh_frame :							\
	{								\
		*(.eh_frame)						\
		*(.eh_frame.*)						\
	} >RAM								\
	__eh_frame_end = .;						\
											\
	__eh_frame_start = ADDR(.eh_frame);								\
	__eh_frame_stop = ALIGN(__eh_frame_start + SIZEOF(.eh_frame), 128);		\
									\
	__eh_frame_hdr_begin = .;					\
	.eh_frame_hdr :							\
	{								\
		*(.eh_frame_hdr)					\
		*(.eh_frame_hdr.*)					\
	} >RAM								\
	__eh_frame_hdr_end = .;			\
									\
	__eh_frame_hdr_start = ADDR(.eh_frame_hdr);								\
	__eh_frame_hdr_stop = ALIGN(__eh_frame_hdr_start + SIZEOF(.eh_frame_hdr), 128);

#define CTORTAB_SECTION							\
	. = ALIGN(__PAGE_SIZE);						\
	uk_ctortab_begin = .;						\
	.uk_ctortab :							\
	{								\
		KEEP(*(SORT_BY_NAME(.uk_ctortab[0-9])))			\
	}								\
	uk_ctortab_end = .;				\
									\
	__start_uk_ctortab = ADDR(.uk_ctortab);								\
	__stop_uk_ctortab = ALIGN(__start_uk_ctortab + SIZEOF(.uk_ctortab), 8);

#define INITTAB_SECTION							\
	uk_inittab_begin = .;						\
	.uk_inittab :							\
	{								\
		KEEP(*(SORT_BY_NAME(.uk_inittab[1-6][0-9])))		\
	}								\
	uk_inittab_end = .;				\
									\
	__start_uk_inittab = ADDR(.uk_inittab);								\
	__stop_uk_inittab = ALIGN(__start_uk_inittab + SIZEOF(.uk_inittab), 8);

#define TLS_SECTIONS							\
	. = ALIGN(0x10);							\
	_tls_begin = .;							\
	.tdata :							\
	{								\
		*(.tdata)						\
		*(.tdata.*)						\
		*(.gnu.linkonce.td.*)					\
	}								\
	_etdata = .;							\
	.tbss :								\
	{								\
		*(.tbss)						\
		*(.tbss.*)						\
		*(.gnu.linkonce.tb.*)					\
		. = ALIGN(0x10);						\
	}								\
	_tls_end = . + SIZEOF(.tbss);	\
									\
	__start_tls = ADDR(.tdata);								\
	__stop_tls = ALIGN(__start_tls + SIZEOF(.tdata) + SIZEOF(.tbss), 128);

#endif /* __UK_COMMON_LDS_H */
