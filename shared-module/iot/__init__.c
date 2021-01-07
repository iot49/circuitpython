/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * SPDX-FileCopyrightText: Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Josef Gajdusek
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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

#include "py/obj.h"
#include "py/objarray.h"
#include "py/binary.h"
#include "py/stream.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "lib/utils/interrupt_char.h"

#include "shared-bindings/iot/__init__.h"


void shared_module_iot_dupterm(mp_obj_t term) {
    MP_STATE_VM(dupterm_objs[0]) = term;
}

mp_obj_t shared_module_iot_terminal() {
    if (MP_STATE_VM(dupterm_objs[0]) == MP_OBJ_NULL) {
        MP_STATE_VM(dupterm_objs[0]) = mp_const_none;
    }
    return MP_STATE_VM(dupterm_objs[0]);
}

STATIC void dupterm_deactivate(const char *msg, mp_obj_t exc) {
    // avoid infinite chain of errors
    MP_STATE_VM(dupterm_objs[0]) = mp_const_none;
    mp_printf(&mp_plat_print, msg);
    if (exc != MP_OBJ_NULL) {
        mp_obj_print_exception(&mp_plat_print, exc);
    }
}

bool common_hal_dupterm_bytes_available() {
    mp_obj_t terminal = MP_STATE_VM(dupterm_objs[0]);
    if (terminal == MP_OBJ_NULL || terminal == mp_const_none) {
        return false;
    }
    mp_obj_t dest[2];
    mp_load_method_maybe(terminal, MP_QSTR_in_waiting, dest);
    if (dest[0] != MP_OBJ_NULL && dest[1] == MP_OBJ_NULL) {
        mp_int_t available = mp_obj_get_int(dest[0]);
        return available > 0;
    }
    return false;
}

STATIC void dupterm_read_write(void *buf, mp_uint_t size, qstr qst) {
    mp_obj_t terminal = MP_STATE_VM(dupterm_objs[0]);
    if (terminal == MP_OBJ_NULL || terminal == mp_const_none) {
        return;
    }
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t dest[3];
        mp_load_method_maybe(terminal, qst, dest);
        if (dest[1] == MP_OBJ_NULL) {
            // method does not exist
            return;
        }
        mp_obj_array_t ar = {{&mp_type_bytearray}, BYTEARRAY_TYPECODE, 0, size, buf};
        dest[2] = MP_OBJ_FROM_PTR(&ar);;
        mp_call_method_n_kw(1, 0, dest);
        nlr_pop();
    } else {
        dupterm_deactivate("dupterm: Exception in read/write, deactivating: ", MP_OBJ_FROM_PTR(nlr.ret_val));
    }
}

char common_hal_dupterm_read(void) {
    char text[1];
    dupterm_read_write(&text, 1, MP_QSTR_readinto);
    return text[0];
}

void common_hal_dupterm_write_substring(uint8_t* text, uint32_t length) {
    dupterm_read_write(text, length, MP_QSTR_write);
}
