/*
 * MIT License
 *
 * Copyright (c) 2018, Sergey Matyukevich <https://github.com/s-matyukevich/raspberry-pi-os>
 *           (c) 2020, Santiago Pagani <santiagopagani@gmail.com>
 * Copyright (c) 2022, John A. Kressel <jkressel.apps@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __MORELLO_MM_H__
#define __MORELLO_MM_H__

/* From https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson06/rpi-os.md:
 * We need just one PGD and one PUD to map the whole RPi memory, and both PGD and PUD will contain a single descriptor.
 * If we have a single PUD entry there also must be a single PMD table, to which this entry will point.
 * Single PMD entry covers 2 MB, there are 512 items in a PMD, so in total the whole PMD table covers the same 1 GB of
 * memory that is covered by a single PUD descriptor.
 * We need to map 1 GB region of memory, which is a multiple of 2 MB, therefore we can use section mapping, and hence we
 * don't need PTE at all.
 * Finally, that means we need three 4 kB pages for the page tables: one for PGD, PUD and PMD.
 */

#define VA_START 		0
#define PHYS_MEMORY_SIZE 	0x3FFFFFFFFFFF

#define PAGE_MASK			0xFFFFFFFFFFFFF000
#define PAGE_SHIFT	 		12
#define TABLE_SHIFT 		9
#define SECTION_SHIFT		(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   		(1 << PAGE_SHIFT)	
#define SECTION_SIZE		(1 << SECTION_SHIFT)	

#define LOW_MEMORY          (2 * SECTION_SIZE)
#define HIGH_MEMORY         DEVICE_BASE

#define PAGING_MEMORY 		(HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES 		(PAGING_MEMORY/PAGE_SIZE)

#define PTRS_PER_TABLE		(1 << TABLE_SHIFT)

#define PGD_SHIFT			PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			PAGE_SHIFT + TABLE_SHIFT

#define PG_DIR_SIZE			(3 * PAGE_SIZE)

/*
 * We will place the pagetable and boot stack after image area,
 * So we define the address offset of pagetable and boot stack
 * here.
 */

/*
 * Each entry in L0_TABLE can link to a L1_TABLE which supports 512GiB
 * memory mapping. One 4K page can provide 512 entries. In this case,
 * one page for L0_TABLE is enough for current stage.
 */
#define L0_TABLE_OFFSET 0
#define L0_TABLE_SIZE   __PAGE_SIZE

/*
 * Each entry in L1_TABLE can map to a 1GiB memory or link to a
 * L2_TABLE which supports 1GiB memory mapping. One 4K page can provide
 * 512 entries. We need at least 2 pages to support 1TB memory space
 * for platforms like KVM QEMU virtual machine.
 */
#define L1_TABLE_OFFSET (L0_TABLE_OFFSET + L0_TABLE_SIZE)
#define L1_TABLE_SIZE   (__PAGE_SIZE * 2)

/*
 * Each entry in L2_TABLE can map to a 2MiB block memory or link to a
 * L3_TABLE which supports 2MiB memory mapping. We need a L3_TABLE to
 * cover image area for us to manager different sections attributes.
 * So, we need one page for L2_TABLE to provide 511 enties for 2MiB
 * block mapping and 1 entry for L3_TABLE link.
 */
#define L2_TABLE_OFFSET (L1_TABLE_OFFSET + L1_TABLE_SIZE)
#define L2_TABLE_SIZE   __PAGE_SIZE

/*
 * We will use Unikraft image's size to caculate the L3_TABLE_SIZE.
 * Because we allocate one page for L2 TABLE, fo the max image size
 * would be 1GB. It would be enough for current stage.
 */
#define L3_TABLE_OFFSET (L2_TABLE_OFFSET + L2_TABLE_SIZE)



#endif /* __MORELLO_MM_H__ */
