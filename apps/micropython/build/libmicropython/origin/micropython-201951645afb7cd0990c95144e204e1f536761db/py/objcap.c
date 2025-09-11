/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 University of Glasgow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//#ifdef __CHERI_PURE_CAPABILITY__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <cheriintrin.h>

#include "py/runtime.h"

typedef struct _mp_obj_cap_t {
    mp_obj_base_t base;
    void * value;
} mp_obj_cap_t;

STATIC void capability_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_cap_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "%p", self->value);
}

STATIC mp_obj_t capability_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    mp_obj_cap_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_BOOL:
            return mp_obj_new_bool(__builtin_cheri_tag_get(self->value));
        case MP_UNARY_OP_HASH:
            return MP_OBJ_NEW_SMALL_INT(__builtin_cheri_address_get(self->value));
        case MP_UNARY_OP_INT_MAYBE:
            return mp_obj_new_int(__builtin_cheri_address_get(self->value));
        default:
            return MP_OBJ_NULL;      // op not supported
    }
}

STATIC mp_obj_t capability_binary_op(mp_binary_op_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
    mp_obj_cap_t *lhs = MP_OBJ_TO_PTR(lhs_in);
    uintptr_t lhs_val = (uintptr_t)lhs->value;
    mp_int_t rhs_val;
    if (!mp_obj_get_int_maybe(rhs_in, &rhs_val)) {
        return MP_OBJ_NULL; // op not supported
    }

    switch (op) {
        case MP_BINARY_OP_OR:
	case MP_BINARY_OP_INPLACE_OR:
	    lhs_val |= rhs_val;
	    break;
        case MP_BINARY_OP_XOR:
	case MP_BINARY_OP_INPLACE_XOR:
	    lhs_val ^= rhs_val;
	    break;
        case MP_BINARY_OP_AND:
	case MP_BINARY_OP_INPLACE_AND:
	    lhs_val &= rhs_val;
	    break;
        case MP_BINARY_OP_ADD:
        case MP_BINARY_OP_INPLACE_ADD:
            lhs_val += rhs_val;
            break;
        case MP_BINARY_OP_SUBTRACT:
        case MP_BINARY_OP_INPLACE_SUBTRACT:
            lhs_val -= rhs_val;
            break;
        case MP_BINARY_OP_LESS:
            return mp_obj_new_bool(lhs_val < (uintptr_t)rhs_val);
        case MP_BINARY_OP_MORE:
            return mp_obj_new_bool(lhs_val > (uintptr_t)rhs_val);
        case MP_BINARY_OP_EQUAL:
            return mp_obj_new_bool(lhs_val == (uintptr_t)rhs_val);
        case MP_BINARY_OP_LESS_EQUAL:
            return mp_obj_new_bool(lhs_val <= (uintptr_t)rhs_val);
        case MP_BINARY_OP_MORE_EQUAL:
            return mp_obj_new_bool(lhs_val >= (uintptr_t)rhs_val);
        default:
            return MP_OBJ_NULL; // op not supported
    }
    return mp_obj_new_cap((void*)lhs_val);
}

STATIC mp_obj_t cap_to_bytes(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    mp_obj_cap_t * self = MP_OBJ_TO_PTR(args[0]);

    mp_int_t len = mp_obj_get_int(args[1]);
    if(len < 0) {
        mp_raise_ValueError(NULL);
    }

    vstr_t vstr;
    // ensure our buffer is properly aligned
    void ** buf = m_new(void*, (len + sizeof(void*)) / sizeof(void*));
    vstr_init_fixed_buf(&vstr, len+1, (void *)buf);
    vstr.len = len;
    vstr.fixed_buf = false; // we don't actually want a fixed buffer, just needed to manually align
    memset(buf, 0, len);
    
    *buf = self->value;

    return mp_obj_new_bytes_from_vstr(&vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(cap_to_bytes_obj, 3, 4, cap_to_bytes);

STATIC const mp_rom_map_elem_t cap_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_to_bytes), MP_ROM_PTR(&cap_to_bytes_obj) },
};

STATIC MP_DEFINE_CONST_DICT(cap_locals_dict, cap_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_cap, MP_QSTR_cap, MP_TYPE_FLAG_EQ_CHECKS_OTHER_TYPE,
    print, capability_print,
    unary_op, capability_unary_op,
    binary_op, capability_binary_op,
    locals_dict, &cap_locals_dict
    );

mp_obj_t mp_obj_new_cap(void * value) {
    mp_obj_cap_t *o = m_new_obj(mp_obj_cap_t);
    o->base.type = &mp_type_cap;
    o->value = value;
    return MP_OBJ_FROM_PTR(o);
}

void * mp_obj_cap_get(mp_obj_t self_in) {
    if(mp_obj_is_type(self_in, &mp_type_cap)) {
        mp_obj_cap_t * self = MP_OBJ_TO_PTR(self_in);
        return self->value;
    } else {
	// allow integer objects to appear as tagless pointers, in particular for NULLs
	return (void*)(uintptr_t)mp_obj_int_get_truncated(self_in);
    }
}


const mp_obj_cap_t mp_const_cap_null_obj = {{&mp_type_cap}, NULL};

//#endif // __CHERI_PURE_CAPABILITY__

