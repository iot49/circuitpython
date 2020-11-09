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
#include "shared-bindings/iot/Chronometer.h"
#include "shared-bindings/time/__init__.h"
#include "shared-bindings/util.h"

// extern uint64_t common_hal_time_monotonic(void);


//| .. currentmodule:: timer
//|
//| :class:`Chronometer` -- Measure elapsed time
//| ====================================================================================
//|
//| Chronometer measures the time since creation or reset.
//|
//| .. class:: Chronometer()
//|
//|   Create a Chronometer and set the initial elapsed time to zero.
//|   Implemented in software. Heap allocation from constructor only.
//|   Milli-second resolution.
//|
//|   Compared to solutions based on `time.monotonic()`, the time resolution
//|   of `Chronometer` is relative to the period measured, not the time
//|   when the microcontroller was started (i.e. it does not suffer from
//|   decreasing accuracy for long power-on times.).
//|
//|   Chronometer behave like stop watches: the clock starts when instantiated,
//|   `stop()`, `resume()`, `reset()` do what the name says. The elapsed time is
//|   is available as attribute `elapsed_time` (float, in seconds).
//|
//|   The `with` statement is also supported and resets the `Chonometer` on entry
//|   and stops on exit.
//|
//|   For example::
//|
//|     from timer import Chronometer
//|     from time import sleep
//|
//|     chrono = Chronometer()
//|     # do some work, e.g.
//|     sleep(0.53)
//|     print("Elapsed time: {} seconds".format(chrono.elapsed_time)
//|
STATIC mp_obj_t timer_chronometer_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_arg_check_num(n_args, kw_args, 0, 0, false);

    timer_chronometer_obj_t *self = m_new_obj(timer_chronometer_obj_t);
    self->base.type = &timer_chronometer_type;
    self->start_time = common_hal_time_monotonic();

    return MP_OBJ_FROM_PTR(self);
}

//|   .. attribute:: elapsed_time
//|
//|     Elapsed time since construction or reset in seconds (float).
//|
STATIC mp_obj_t timer_chronometer_obj_get_elapsed_time(mp_obj_t self_in) {
    timer_chronometer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    float time_delta;
    if (self->start_time > 0) {
        // running
        time_delta = (int64_t)common_hal_time_monotonic() - self->start_time;
    } else {
        // stopped. start_time is elapsed time so far, negative.
        time_delta = -self->start_time;
    }
    return mp_obj_new_float(time_delta/1000.0);
}
MP_DEFINE_CONST_FUN_OBJ_1(timer_chronometer_get_elapsed_time_obj, timer_chronometer_obj_get_elapsed_time);

//|   .. method:: stop
//|
//|     Stop clock.
//|
STATIC mp_obj_t timer_chronometer_obj_stop(mp_obj_t self_in) {
    timer_chronometer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->start_time > 0)
        self->start_time = self->start_time - (int64_t)common_hal_time_monotonic();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(timer_chronometer_stop_obj, timer_chronometer_obj_stop);

//|   .. method:: resume
//|
//|     Resume clock.
//|
STATIC mp_obj_t timer_chronometer_obj_resume(mp_obj_t self_in) {
    timer_chronometer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->start_time < 0)
        self->start_time = self->start_time + common_hal_time_monotonic();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(timer_chronometer_resume_obj, timer_chronometer_obj_resume);

//|   .. method:: reset
//|
//|     Reset chronometer to restart measuring from present time.
//|
STATIC mp_obj_t timer_chronometer_obj_reset(mp_obj_t self_in) {
    timer_chronometer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->start_time = common_hal_time_monotonic();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(timer_chronometer_reset_obj, timer_chronometer_obj_reset);

//|   .. method:: __enter__()
//|
//|      Reset chronometer.
//|
STATIC mp_obj_t timer_chronometer_obj___enter__(mp_obj_t self_in) {
    timer_chronometer_obj_reset(self_in);
    return self_in;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_chronometer___enter___obj, timer_chronometer_obj___enter__);

//|   .. method:: __exit__()
//|
//|      Stop chronometer. Property `elapsed_time` returns the elapsed time.
//|
STATIC mp_obj_t timer_chronometer_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    return timer_chronometer_obj_stop(args[0]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(timer_chronometer___exit___obj, 4, 4, timer_chronometer_obj___exit__);

const mp_obj_property_t timer_chronometer_elapsed_time_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&timer_chronometer_get_elapsed_time_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC const mp_rom_map_elem_t timer_chronometer_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&timer_chronometer___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&timer_chronometer___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&timer_chronometer_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_resume), MP_ROM_PTR(&timer_chronometer_resume_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&timer_chronometer_reset_obj) },
    // Properties
    { MP_ROM_QSTR(MP_QSTR_elapsed_time), MP_ROM_PTR(&timer_chronometer_elapsed_time_obj) },
};
STATIC MP_DEFINE_CONST_DICT(timer_chronometer_locals_dict, timer_chronometer_locals_dict_table);

const mp_obj_type_t timer_chronometer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Chronometer,
    .make_new = timer_chronometer_make_new,
    .locals_dict = (mp_obj_dict_t*)&timer_chronometer_locals_dict,
};
