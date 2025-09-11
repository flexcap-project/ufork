#ifndef SHARED
#include <stddef.h>
#include <stdint.h>
#include <sys/auxv.h>
#include "elf.h"
#include "cap_perms.h"
#include "reloc.h"
#endif

__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
#ifndef SHARED
"	mov c20, c0\n"
"	mov c21, c1\n"
"	mov c22, c2\n"
"	mov c23, c3\n"
"	bl __morello_init_static\n"
#else
"	mov c5, c3\n"
"	mov c4, c2\n"
"	mov c3, c1\n"
"	mov c2, c0\n"
#endif
"	mov c0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp c1, _DYNAMIC\n"
"	add c1, c1, #:lo12:_DYNAMIC\n"
"	alignd csp, csp, #4\n"
#ifndef SHARED
"	mov c2, c20\n"
"	mov c3, c21\n"
"	mov c4, c22\n"
"	mov c5, c23\n"
#endif
"	b " START "_c\n"
".size " START ", .-" START "\n"
);

#ifndef SHARED

// #ifdef CAP_RELOCS
typedef struct {
	uint64_t location;	/* Capability location */
	uint64_t base;		/* Object referred to by the capability */
	size_t offset;		/* Offset in the object */
	size_t size;		/* Size */
	size_t permissions;	/* Inverted permissions mask */
} cap_relocs_entry;

inline static void
__do_morello_cap_relocs(void *rw, void *rx)
{
	cap_relocs_entry *__cap_relocs_start, *__cap_relocs_end;

	__asm__ (
		".p2align 4\n"
		".weak __cap_relocs_start\n"
		"adrp 	%0, __cap_relocs_start\n"
		"add	%0, %0, :lo12:__cap_relocs_start\n"
		".weak __cap_relocs_end\n"
		"adrp 	%1, __cap_relocs_end\n"
		"add	%1, %1, :lo12:__cap_relocs_end" : "=C"(__cap_relocs_start), "=C"(__cap_relocs_end));

	for (const cap_relocs_entry *r = __cap_relocs_start; r != __cap_relocs_end; r++) {
		if (r->base) { // if capability is not null
			void *cap = NULL;
			size_t perm = ~r->permissions;
			_Bool is_fun_ptr = perm & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__;
			_Bool is_writable = perm & __CHERI_CAP_PERMISSION_PERMIT_STORE__;
			if (is_writable) {
				cap = __builtin_cheri_address_set(rw, r->base);
			} else {
				cap = __builtin_cheri_address_set(rx, r->base);
			}
			cap = __builtin_cheri_perms_and(cap, perm);
			cap = __builtin_cheri_bounds_set_exact(cap, r->size);
			cap = __builtin_cheri_offset_set(cap, r->offset);
			if (is_fun_ptr) {
				// RB-seal function pointer
				cap = __builtin_cheri_seal_entry(cap);
			}
			// store capability
			void **loc = __builtin_cheri_address_set(rw, r->location);
			loc = __builtin_cheri_bounds_set_exact(loc, sizeof(void *));
			*loc = cap;
		}
	}
}

inline static void
__do_morello_relative_relocs(void *rw, void *rx)
{
	Elf64_Rela *__rela_dyn_start, *__rela_dyn_end;
	__asm__ (
		".p2align 4\n"
		".weak __rela_dyn_start\n"
		".hidden __rela_dyn_start\n"
		"adrp 	%0, __rela_dyn_start\n"
		"add	%0, %0, :lo12:__rela_dyn_start\n"
		".weak __rela_dyn_end\n"
		".hidden __rela_dyn_end\n"
		"adrp 	%1, __rela_dyn_end\n"
		"add	%1, %1, :lo12:__rela_dyn_end"  : "=C"(__rela_dyn_start), "=C"(__rela_dyn_end));

	for (const Elf64_Rela *r = __rela_dyn_start; r != __rela_dyn_end; r++) {
		if (r->r_info != R_MORELLO_RELATIVE) continue;
		void *cap = NULL;
		void **loc = __builtin_cheri_address_set(rw, r->r_offset);
		const morello_reloc_cap_t *u = (morello_reloc_cap_t *)loc;
		switch (u->perms) {
			case MORELLO_RELA_PERM_R:
				cap = __builtin_cheri_perms_and(rx, __CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS);
				break;
			case MORELLO_RELA_PERM_RW:
				cap = __builtin_cheri_perms_and(rw, __CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS | WRITE_CAP_PERMS);
				break;
			case MORELLO_RELA_PERM_RX:
				cap = __builtin_cheri_perms_and(rx, __CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS | EXEC_CAP_PERMS);
				break;
			default:
				cap = __builtin_cheri_perms_and(rx, 0);
				break;
		}
		cap = __builtin_cheri_address_set(cap, u->address);
		cap = __builtin_cheri_bounds_set_exact(cap, u->length);
		cap = cap + r->r_addend;
		if (u->perms == MORELLO_RELA_PERM_RX) {
			cap = __builtin_cheri_seal_entry(cap);
		}
		*loc = cap;
	}
}

void
__morello_init_static(int argc, char **argv, char **envp, auxv_entry *auxv)
{
	void *rw = NULL, *rx = NULL;
	size_t phnum = 0, phent = 0;
	Elf64_Phdr *ph = NULL;
	for (; auxv->a_type; auxv++) {
		if (auxv->a_type == AT_CHERI_EXEC_RW_CAP) {
			rw = auxv->a_un.a_ptr; // used to derive read-only and rw objects
		} else if (auxv->a_type == AT_CHERI_EXEC_RX_CAP) {
			rx = auxv->a_un.a_ptr; // used to derive function pointers
		} else if (auxv->a_type == AT_PHDR) {
			ph = auxv->a_un.a_ptr;
		} else if (auxv->a_type == AT_PHNUM) {
			phnum = auxv->a_un.a_val;
		} else if (auxv->a_type == AT_PHENT) {
			phent = auxv->a_un.a_val;
		}
		if (rw && rx && ph && phnum && phent) {
			break;
		}
	}

	while(phnum--) {
		if (ph->p_type == PT_INTERP) {
			return;
		}
		ph = (void *)((char *)ph + phent);
	}

	__do_morello_cap_relocs(rw, rx);
	__do_morello_relative_relocs(rw, rx);
}
#endif
