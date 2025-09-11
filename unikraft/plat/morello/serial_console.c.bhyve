/*
 * Copyright (C) 2018, bzt (bztsrc@github), https://github.com/bztsrc/raspi3-tutorial
 * Copyright (C) 2020, Santiago Pagani <santiagopagani@gmail.com>
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

#include <morello/sysregs.h>
#include <stdint.h>

/* PL011 UART registers */
#define UART0_DR        ((int)(0))
#define UART0_FR        ((int)(2))
#define UART0_IBRD      ((int)(4))
#define UART0_FBRD      ((int)(5))
#define UART0_LCRH      ((int)(6))
#define UART0_CR        ((int)(7))
#define UART0_IMSC      ((int)(9))
#define UART0_ICR       ((int)(12))

volatile unsigned int* uart_cap;

// #define morello_create_capability_from_ptr(ptr, size, store_to_ptr)	\
// 	__asm__ volatile(	\
// 		"cvtp c0, %0\n"	\
// 		"scbnds c0, c0, %2\n"	\
// 		"mov x1, #(1<<1)\n"	\
// 		"clrperm c0, c0, x1\n"	\
// 		"str c0, [%1]\n"	\
// 		:	\
// 		: "r"((uintptr_t *)(ptr)), "r"((uintptr_t *)(store_to_ptr)), "r"(size)	\
// 		: "c0", "x1", "memory"	\
// 	)

static char prev_sent_char = '\0';

//void *__capability uart_cap;

static void wait_cycles(unsigned int n)
{
    if (n) {
		while (n--) {
			asm volatile("nop");
		}
	}
}

unsigned int serial_tx_buffer_full(void)
{
	//return *(uart_cap+UART0_FR)&0x20;
  uint32_t result = 0;
  __asm__ volatile(
    "mov w0, %2 \n"
    "ldr w1, [%0, #0x18]\n"
    "and w1, w1, w0\n"
    "str w1, [%1]\n"
    :
    : "r"(uart_cap), "r"(&result), "i"(0x20)
    : "x0", "x1"
  );
  return result;
}

static unsigned int serial_rx_buffer_empty(void)
{
	return *(uart_cap+UART0_FR)&0x10;
}

/**
 * Set baud rate and characteristics (115200 8N1)
 */
void _libmorelloplat_init_serial_console()
{
    unsigned int* defcap = __builtin_cheri_global_data_get();
    defcap = __builtin_cheri_offset_set(defcap, 0x10000);
    uart_cap = (unsigned int*)__builtin_cheri_bounds_set(defcap, 13*sizeof(int));

    /* initialize UART */
    //*(uart_cap+UART0_CR) = 0; // turn off UART0

    //*(uart_cap+UART0_ICR) = 0x7FF;    // clear interrupts
    /*
    *
    * IBRD = 27
    * FBRD = 9
    *
    * Calculation:
    *   UART_CLK = 50MHz
    *   Baud rate divisor = (50 x (10^6)) / (16 * 115200) = 27.12673611
    *
    *   Therefore, IBRD is 27, for the fractional part, we do the following:
    *     ((0.12673611 * 64) + 0.5) = 9 (as integer)
    *   So FBRD is 9
    * 
    *
    */

  //    __asm__ volatile(
  //   "mov w0, %2 \n"
  //   "mov w1, %3 \n"
  //    "mov w2, %4 \n"
  //     "mov w3, %5 \n"
  //     "mov w4, %6 \n"
  //   "str wzr, [%0, 0x30]\n"
  //   "str w4, [%0, 0x44]\n"
  //   "str w0, [%0, 0x24]\n"
  //   "str w1, [%0, 0x28]\n"
  //   "str w2, [%0, 0x2C]\n"
  //   "str w3, [%0, 0x30]\n"
  //   :
  //   : "r"(uart_cap), "i"(3), "i"(0x1B), "i"(0x9), "i"(0b11<<5), "i"(0x301), "i"(0x7FF)
  //   : "x0", "x1", "x2", "x3", "x4"
  // );
    //*(uart_cap+UART0_IBRD) = 0x1B;       // 115200 baud
    //*(uart_cap+UART0_FBRD) = 0x9;
    //*(uart_cap+UART0_LCRH) = 0b11<<5; // 8n1
    //*(uart_cap+UART0_CR) = 0x301;     // enable Tx, Rx, FIFO

    //morello_create_capability_from_ptr((uintptr_t)0x2A400000, (uintptr_t)0x60, ((uintptr_t *)(&(uart_cap))));

}

/**
 * Send a character
 */
void _libmorelloplat_serial_putc(char c)
{
	if ((c == '\n') && (prev_sent_char != '\r'))
		_libmorelloplat_serial_putc('\r');

    // Wait until we can send
    do{
		asm volatile("nop");
	} while (serial_tx_buffer_full());

  //      int doit = 0;
  //  while(doit < 10000000) {
  //    asm volatile ("nop\n");
  //    doit++;
  //  }

    // Write the character to the buffer
       __asm__ volatile(
    "mov w0, %w1 \n"
    "str w0, [%0]\n"
    :
    : "r"(uart_cap), "r"(c)
    : "x0"
  );
  //  *(uart_cap+UART0_DR) = c;
  // __asm__ volatile(
  //   "mov x0, %1 \n"
  //   "str x0, [%0]\n"
  //   :
  //   : "r"(uart_cap), "r"(c)
  //   : "x0"
  // );
	prev_sent_char = c;
}

/**
 * Receive a character
 */
int  _libmorelloplat_serial_getc(void)
{
	if (serial_rx_buffer_empty())
		return -1;

    char r;
    r = (char)(*(uart_cap+UART0_DR));
    return (int)r;
}
