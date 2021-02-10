/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * SPDX-FileCopyrightText: Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Josef Gajdusek
 * Copyright (c) 2016 Scott Shawcroft for Adafruit Industries
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

#include "py/mpconfig.h"
#include "py/smallint.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "shared-bindings/time/__init__.h"

//| """time and timing related functions
//|
//| The `utime` module adds wrap-around time functions for compatibility with MicroPython."""
//|
//| def ticks_ms() -> int:
//|     """Returns wrap-around time in [ms]."""
//|     ...
//|
STATIC mp_obj_t utime_ticks_ms(void) {
    uint64_t ticks_ms = common_hal_time_monotonic_ms();
    return MP_OBJ_NEW_SMALL_INT((int32_t)ticks_ms & (MICROPY_PY_UTIME_TICKS_PERIOD - 1));
}
MP_DEFINE_CONST_FUN_OBJ_0(utime_ticks_ms_obj, utime_ticks_ms);

//| def ticks_us() -> int:
//|     """Returns wrap-around time in [us]."""
//|     ...
//|
STATIC mp_obj_t utime_ticks_us(void) {
    uint64_t ticks_us = common_hal_time_monotonic_ns() / 1000;
    return MP_OBJ_NEW_SMALL_INT((int32_t)ticks_us & (MICROPY_PY_UTIME_TICKS_PERIOD - 1));
}
MP_DEFINE_CONST_FUN_OBJ_0(utime_ticks_us_obj, utime_ticks_us);

//| def ticks_diff(t1:int, t2:int) -> int:
//|     """Returns t1-t2.
//|        Assumes that t1 and t2 are wrap-around times generated with ticks_ms or ticks_us."""
//|     ...
//|
STATIC mp_obj_t utime_ticks_diff(mp_obj_t end_in, mp_obj_t start_in) {
    // we assume that the arguments come from ticks_xx so are small ints
    mp_uint_t start = MP_OBJ_SMALL_INT_VALUE(start_in);
    mp_uint_t end = MP_OBJ_SMALL_INT_VALUE(end_in);
    // Optimized formula avoiding if conditions. We adjust difference "forward",
    // wrap it around and adjust back.
    mp_int_t diff = ((end - start + MICROPY_PY_UTIME_TICKS_PERIOD / 2) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1))
        - MICROPY_PY_UTIME_TICKS_PERIOD / 2;
    return MP_OBJ_NEW_SMALL_INT(diff);
}
MP_DEFINE_CONST_FUN_OBJ_2(utime_ticks_diff_obj, utime_ticks_diff);

//| def ticks_add(t1:int, t2:int) -> int:
//|     """Returns t1+t2.
//|        Assumes that t1 and t2 are wrap-around times generated with ticks_ms or ticks_us."""
//|     ...
//|
STATIC mp_obj_t utime_ticks_add(mp_obj_t ticks_in, mp_obj_t delta_in) {
    // we assume that first argument come from ticks_xx so is small int
    mp_uint_t ticks = MP_OBJ_SMALL_INT_VALUE(ticks_in);
    mp_uint_t delta = mp_obj_get_int(delta_in);
    return MP_OBJ_NEW_SMALL_INT((ticks + delta) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1));
}
MP_DEFINE_CONST_FUN_OBJ_2(utime_ticks_add_obj, utime_ticks_add);

//| def sleep_ms(dt:int) -> None:
//|     """Delay execution for dt milli-seconds."""
//|     ...
//|
STATIC mp_obj_t utime_sleep_ms(mp_obj_t arg) {
    mp_int_t ms = mp_obj_get_int(arg);
    if (ms > 0) {
        common_hal_time_delay_ms(ms);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(utime_sleep_ms_obj, utime_sleep_ms);

STATIC const mp_rom_map_elem_t utime_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),   MP_ROM_QSTR(MP_QSTR_utime) },

    { MP_ROM_QSTR(MP_QSTR_ticks_ms),   MP_ROM_PTR(&utime_ticks_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_us),   MP_ROM_PTR(&utime_ticks_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_diff), MP_ROM_PTR(&utime_ticks_diff_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_add),  MP_ROM_PTR(&utime_ticks_add_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep_ms),   MP_ROM_PTR(&utime_sleep_ms_obj) },
};

STATIC MP_DEFINE_CONST_DICT(utime_module_globals, utime_module_globals_table);

const mp_obj_module_t utime_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&utime_module_globals,
};
