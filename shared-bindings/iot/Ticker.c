/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Bernhard Boser
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

#include <stdint.h>

#include "lib/utils/context_manager_helpers.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/runtime0.h"
#include "shared-bindings/iot/Ticker.h"
#include "shared-bindings/time/__init__.h"
#include "shared-bindings/util.h"


//| .. currentmodule:: iot
//|
//| :class:`Ticker` -- Generate regular time intervals
//| ====================================================================================
//|
//| Ticker generates regular time intervals. It is typically used with TimeQueue.
//|
//| .. class:: Ticker(period, offset=0)
//|
//|   Create a Ticker with the specified period in seconds.
//|
//|   param float period: ticker period.
//|   param float offset: start time (default: now).
//|
STATIC mp_obj_t iot_ticker_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_period, ARG_offset };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_period, MP_ARG_OBJ | MP_ARG_REQUIRED },
        { MP_QSTR_offset, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(0)} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_float_t period = mp_obj_get_float(args[ARG_period].u_obj);
    mp_float_t offset = mp_obj_get_float(args[ARG_offset].u_obj);

    iot_ticker_obj_t *self = m_new_obj(iot_ticker_obj_t);
    self->base.type = &iot_ticker_type;
    self->period = (long)(1e9*period);
    self->start_time = common_hal_time_monotonic_ns() + (long)(offset);

    return MP_OBJ_FROM_PTR(self);
}

//|   .. attribute:: next_time
//|
//|     Time to next occurance in seconds (float).
//|
STATIC mp_obj_t iot_ticker_obj_get_next_time(mp_obj_t self_in) {
    iot_ticker_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint64_t now = common_hal_time_monotonic_ns();
    uint64_t period = self->period;
    // avoid 64-bit remainder calculation
    while (self->start_time < now) self->start_time += period;
    return mp_obj_new_float(0.001*(self->start_time - now));
}
MP_DEFINE_CONST_FUN_OBJ_1(iot_ticker_get_next_time_obj, iot_ticker_obj_get_next_time);

const mp_obj_property_t iot_ticker_next_time_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&iot_ticker_get_next_time_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC const mp_rom_map_elem_t iot_ticker_locals_dict_table[] = {
    // Properties
    { MP_ROM_QSTR(MP_QSTR_next_time), MP_ROM_PTR(&iot_ticker_next_time_obj) },
};
STATIC MP_DEFINE_CONST_DICT(iot_ticker_locals_dict, iot_ticker_locals_dict_table);

const mp_obj_type_t iot_ticker_type = {
    { &mp_type_type },
    .name = MP_QSTR_Ticker,
    .make_new = iot_ticker_make_new,
    .locals_dict = (mp_obj_dict_t*)&iot_ticker_locals_dict,
};
