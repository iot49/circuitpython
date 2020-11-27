/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include "py/obj.h"
#include "py/runtime.h"

#include "shared-bindings/iot/__init__.h"
#include "shared-bindings/iot/Chronometer.h"
#include "shared-bindings/iot/TimeQueue.h"
#include "shared-bindings/iot/Ticker.h"

//| :mod:`iot` --- Miscellaneous classes and functions
//| ==================================================
//|
//| .. module:: iot
//|   :synopsis: Miscellaneous
//|   :platform: all
//|
//| The `iot` module provides class `Chronometer` for measuring time.
//|
//| Libraries
//|
//| .. toctree::
//|     :maxdepth: 3
//|
//|     Timer
//|     Chronometer
//|     TimeQueue
//|     Ticker
//|
//| Timers should be deinitialized when no longer needed to free up resources.
//|


//| def dupterm(io: terminal) -> None:
//|     """Add/remove secondary terminal."""
//|     ...
//|
mp_obj_t iot_dupterm(size_t n_args, const mp_obj_t *args) {
    if (n_args == 1) shared_module_iot_dupterm(args[0]);
    return shared_module_iot_terminal();
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(iot_dupterm_obj, 0, 1, iot_dupterm);

STATIC const mp_rom_map_elem_t iot_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_iot) },
    { MP_ROM_QSTR(MP_QSTR_Chronometer), MP_ROM_PTR(&timer_chronometer_type) },
    { MP_ROM_QSTR(MP_QSTR_TimeQueue), MP_ROM_PTR(&timer_time_queue_type) },
    { MP_ROM_QSTR(MP_QSTR_Ticker), MP_ROM_PTR(&timer_ticker_type) },

    { MP_ROM_QSTR(MP_QSTR_dupterm), MP_ROM_PTR(&iot_dupterm_obj) },
};

STATIC MP_DEFINE_CONST_DICT(iot_module_globals, iot_module_globals_table);

const mp_obj_module_t iot_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&iot_module_globals,
};
