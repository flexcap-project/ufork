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

#include <stdint.h>
#include <string.h>

#define R_MORELLO_RELATIVE 59395

#define EXEC_PERMS 0x13DBCULL
#define DESC_PERMS 0x13DBCULL
#define RW_PERMS 0x8FBEULL
#define RO_PERMS 0x1BFBEULL


#if defined(__CHERI__)
/* Newlib doesn't have elf.h so define Elf64_Rela here. */
typedef struct {
  uint64_t   r_offset;
  uint64_t   r_info;
  int64_t    r_addend;
} Elf64_Rela;

typedef struct {
  uint64_t cap_location;
  uint64_t base;
  uint64_t offset;
  uint64_t size;
  uint64_t permissions;
} CapReloc;

/* Various functions to get symbol locations. */
void *__getRelStart();
void *__getRelEnd();
void *__getCapRelocsStart();
void *__getCapRelocsEnd();

/* Fragment decoding helpers */
static uint64_t getVA(uint64_t FragLo, uint64_t FragHi) {
  return FragLo;
}

static uint64_t getPerms(uint64_t FragLo, uint64_t FragHi) {
  switch (FragHi >> 56) {
    case 0x4:
      return ~EXEC_PERMS;
    case 0x2:
      return ~RW_PERMS;
    default:
      return ~RO_PERMS;
  }
}

static int isExecutable(uint64_t FragLo, uint64_t FragHi) {
  return (FragHi >> 56) & 0x4ULL;;
}

static uint64_t getLen(uint64_t FragLo, uint64_t FragHi) {
  return FragHi & 0xffffffffffffff;
}

/* Process legacy __cap_relocs */
void __init_global_caps() {
  CapReloc *Start = __getCapRelocsStart();
  CapReloc *End = __getCapRelocsEnd();
  void *__capability DDC = __builtin_cheri_global_data_get();
  void * __capability SealingCap = DDC;
  uint64_t PermsMask =~(__ARM_CAP_PERMISSION_COMPARTMENT_ID__  |
                        __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__ |
                        __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ |
                        __CHERI_CAP_PERMISSION_PERMIT_SEAL__);
  if (Start >= End)
    return;

  DDC = __builtin_cheri_perms_and(DDC, PermsMask);
  for (CapReloc *Ptr = Start; Ptr < End; ++Ptr) {
    uint64_t Location = Ptr->cap_location;
    uint64_t Base = Ptr->base;
    uint64_t Offset = Ptr->offset;
    uint64_t Size = Ptr->size;
    uint64_t Perms = Ptr->permissions;
    void * __capability *__capability Addr;
    void * __capability Cap, * __capability TypeCap;
    /* Executable capabilities have type 1. */
    uint64_t Type =
        (Perms & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__)  == 0 ? 1 : 0;
    Addr = __builtin_cheri_address_set(DDC, Location);
    Cap = __builtin_cheri_address_set(DDC, Base);
    Cap = __builtin_cheri_bounds_set(Cap, Size);
    Cap = Cap + Offset;
    Cap = __builtin_cheri_perms_and(Cap, ~Perms);
    TypeCap = __builtin_cheri_address_set(SealingCap, Type);
    Cap = __builtin_cheri_seal(Cap, TypeCap);
    *Addr = Cap;
  }
}

/* Process dynamic relocations */
void __processRelocs() {
  Elf64_Rela *Start = __getRelStart();
  Elf64_Rela *End = __getRelEnd();
  if (Start >= End)
    return;

  Elf64_Rela *Current = Start;
  void *__capability ddc = __builtin_cheri_global_data_get();
  void * __capability SealingCap = ddc;
  uint64_t PermsMask =~(__ARM_CAP_PERMISSION_COMPARTMENT_ID__  |
                        __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__ |
                        __CHERI_CAP_PERMISSION_PERMIT_SEAL__);
  ddc = __builtin_cheri_perms_and(ddc, PermsMask);

  for (Current = Start; Current < End; ++Current) {
    void *Fragment =
#if defined(__CHERI_PURE_CAPABILITY__)
      __builtin_cheri_address_set(Start, Current->r_offset);
#else
      (void *)Current->r_offset;
#endif
    uint64_t FragLo = *(uint64_t*)Fragment;
    uint64_t FragHi = *(uint64_t*)(Fragment + sizeof(uint64_t));
    uint64_t VA = getVA(FragLo, FragHi);
    uint64_t Len = getLen(FragLo, FragHi);

    switch (Current->r_info) {
      case R_MORELLO_RELATIVE: {
        void *__capability Val = __builtin_cheri_address_set(ddc, VA);
        uint64_t Type = isExecutable(FragLo, FragHi) ? 1 : 0;
        void *__capability TypeCap;
        Val = __builtin_cheri_bounds_set(Val, Len);
        Val = Val + Current->r_addend;
        Val = __builtin_cheri_perms_and(Val, getPerms(FragLo, FragHi));
        TypeCap = __builtin_cheri_address_set(SealingCap, Type);
        Val = __builtin_cheri_seal(Val, TypeCap);
        *(void *__capability*)Fragment = Val;
        continue;
      }
      default:
        continue;
    }
  }
}
#endif
