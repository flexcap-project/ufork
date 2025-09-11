/* Copyright (c) 2022 Arm Ltd.  All rights reserved.

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
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

extern char *__UART;
static char *uartptr = &__UART;

#define UARTFR                  0x018
#define PL011_UARTFR_TXFF_BIT	5	/* Transmit FIFO full bit in UARTFR register */
#define PL011_UARTFR_RXFE_BIT	4	/* Receive FIFO empty bit in UARTFR register */
#define PL011_UARTFR_BUSY_BIT	3	/* UART busy bit in UARTFR register */

/* ptr, is a null terminated string.
   len, is the length in bytes to read.
   Returns the number of bytes *not* written. */
int
__doUARTRead (char *ptr, int len)
{
  /* Not implemented. */
  return 0;
}

static void putchar(volatile char *uart, unsigned char c)
{
  // Prepend '\r' to \n
  if (c == 0xa)
    putchar(uart, 0xd);
  // Wait for space in the transmit FIFO.
  while (1) {
    unsigned status = *(volatile unsigned *)(uart + UARTFR);
    if ((status & (1 << PL011_UARTFR_TXFF_BIT)) == 0)
      break;
  }
  // Write the value to the uart
  *(volatile unsigned *)(uart) = (unsigned)c;
}

/* Returns the number of bytes *not* written. */
int
__doUARTWrite(char *ptr, int len)
{
  volatile char *addr = uartptr;
  for (int i = 0; i < len; ++i) {
    putchar(addr, *ptr);
    ptr++;
  }
  return 0;
}
