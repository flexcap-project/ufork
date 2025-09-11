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

#if defined(__CHERI__)

.text

#if defined(__CHERI_PURE_CAPABILITY__)
#define REG c0
#else
#define REG x0
#endif

  .weak   __cap_relocs_start
  .weak   __cap_relocs_end
  .weak   __rela_dyn_start
  .weak   __rela_dyn_end

  .align 4
.LCRStart:
  .dword __cap_relocs_start
  .globl __getCapRelocsStart
  .type  __getCapRelocsStart,@function
  .align 2
__getCapRelocsStart:
  adr REG, .LCRStart
  ldr x0, [REG]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret

  .align 4
.LCREnd:
  .dword __cap_relocs_end
  .globl __getCapRelocsEnd
  .type  __getCapRelocsEnd,@function
  .align 2
__getCapRelocsEnd:
  adr REG, .LCREnd
  ldr x0, [REG]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret

  .align 4
.LRelStart:
  .dword __rela_dyn_start
  .globl __getRelStart
  .type  __getRelStart,@function
  .align 2
__getRelStart:
  adr REG, .LRelStart
  ldr x0, [REG]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret

  .align 4
.LRelEnd:
  .dword __rela_dyn_end
  .globl __getRelEnd
  .type  __getRelEnd,@function
  .align 2
__getRelEnd:
  adr REG, .LRelEnd
  ldr x0, [REG]
  mrs c1, DDC
  scvalue c0, c1, x0
  ret
#endif
