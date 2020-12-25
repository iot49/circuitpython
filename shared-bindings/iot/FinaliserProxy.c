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

#include "py/runtime.h"
#include "shared-bindings/iot/FinaliserProxy.h"

#define MP_OBJ_IS_METH(o) (MP_OBJ_IS_OBJ(o) && (((mp_obj_base_t*)MP_OBJ_TO_PTR(o))->type->name == MP_QSTR_bound_method))


//| .. currentmodule:: iot
//|
//| :class:`FinaliserProxy` -- ...
//| ====================================================================================
//|
//| FinaliserProxy ...
//|
//| .. class:: FinaliserProxy(callback)
//|
//|   Example:
//|
//|   class FP(FinaliserProxy):
//|       def __init__(self, cb, desc):
//|           self.desc = desc
//|           super().__init__(self.cleanup)
//|
//|       def cleanup(self, arg):
//|           print("cleanup:", self.desc)
//|
//|   f = FP(None, "my custom class with finalizer")
//|
//|   print(f)
//|
//|   # ... use f, then
//|   f = None   # or del f
//|   # When the gc collects f, cleanup is called.


STATIC mp_obj_t iot_finaliser_proxy_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ | MP_ARG_REQUIRED },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t callback = args[ARG_callback].u_obj;

    if (!MP_OBJ_IS_FUN(callback) && !MP_OBJ_IS_METH(callback)) {
        mp_raise_ValueError(translate("function expected"));
    }

    iot_finaliser_proxy_obj_t *self = m_new_obj_with_finaliser(iot_finaliser_proxy_obj_t);
    self->base.type = &iot_finaliser_proxy_type;
    self->callback = callback;

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t iot_finaliser_proxy_cleanup(mp_obj_t self_in) {
    iot_finaliser_proxy_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // return value intentionally discarded
    mp_call_function_0(self->callback);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(iot_finaliser_proxy_cleanup_obj, iot_finaliser_proxy_cleanup);

STATIC const mp_rom_map_elem_t iot_finaliser_proxy_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&iot_finaliser_proxy_cleanup_obj) },
};
STATIC MP_DEFINE_CONST_DICT(iot_finaliser_proxy_locals_dict, iot_finaliser_proxy_locals_dict_table);

const mp_obj_type_t iot_finaliser_proxy_type = {
    { &mp_type_type },
    .name = MP_QSTR_FinaliserProxy,
    .make_new = iot_finaliser_proxy_make_new,
    .locals_dict = (mp_obj_dict_t*)&iot_finaliser_proxy_locals_dict,
};
