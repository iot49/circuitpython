/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
 * Copyright (c) 2016-2017 Paul Sokolovsky
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

#include <string.h>

#include "py/objlist.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/smallint.h"

#include "supervisor/shared/translate.h"
#include "shared-bindings/time/__init__.h"

#define DEBUG 0

// the algorithm here is modelled on CPython's heapq.py

struct qentry {
    uint64_t time;
    mp_obj_t callback;
};

typedef struct _mp_obj_time_queue_t {
    mp_obj_base_t base;
    mp_uint_t alloc;
    mp_uint_t len;
    struct qentry items[];
} mp_obj_time_queue_t;

STATIC mp_obj_time_queue_t *get_heap(mp_obj_t self_in) {
    return MP_OBJ_TO_PTR(self_in);
}

STATIC bool time_less_than(struct qentry *item, struct qentry *parent) {
    return item->time < parent->time;
}

//| .. currentmodule:: iot
//|
//| :class:`TimeQueue` -- Queue with items sorted by time.
//| ====================================================================================
//|
//| TimeQueue supports scheduling at predetermined times.
//|
//| .. class:: TimeQueue(max_length)
//|
//|   Create a TimeQueue with the specified capacity. Present implementation is
//|   backed by common_hal_time_monotonic().
//|
//|   param int max_length: maximum number of items the queue can hold.
//|
STATIC mp_obj_t time_queue_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_length };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_length, MP_ARG_INT | MP_ARG_REQUIRED },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    int length  = args[ARG_length].u_int;
    if (length < 1) length = 1;

    mp_obj_time_queue_t *self = m_new_obj_var(mp_obj_time_queue_t, struct qentry, length);
    self->base.type = type;
    memset(self->items, 0, sizeof(*self->items) * length);
    self->alloc = length;
    self->len = 0;
    return MP_OBJ_FROM_PTR(self);
}

STATIC void heap_siftdown(mp_obj_time_queue_t *heap, mp_uint_t start_pos, mp_uint_t pos) {
    struct qentry item = heap->items[pos];
    while (pos > start_pos) {
        mp_uint_t parent_pos = (pos - 1) >> 1;
        struct qentry *parent = &heap->items[parent_pos];
        bool lessthan = time_less_than(&item, parent);
        if (lessthan) {
            heap->items[pos] = *parent;
            pos = parent_pos;
        } else {
            break;
        }
    }
    heap->items[pos] = item;
}

STATIC void heap_siftup(mp_obj_time_queue_t *heap, mp_uint_t pos) {
    mp_uint_t start_pos = pos;
    mp_uint_t end_pos = heap->len;
    struct qentry item = heap->items[pos];
    for (mp_uint_t child_pos = 2 * pos + 1; child_pos < end_pos; child_pos = 2 * pos + 1) {
        // choose right child if it's <= left child
        if (child_pos + 1 < end_pos) {
            bool lessthan = time_less_than(&heap->items[child_pos], &heap->items[child_pos + 1]);
            if (!lessthan) {
                child_pos += 1;
            }
        }
        // bubble up the smaller child
        heap->items[pos] = heap->items[child_pos];
        pos = child_pos;
    }
    heap->items[pos] = item;
    heap_siftdown(heap, start_pos, pos);
}

//|   .. method:: after(delay, item)
//|
//|     Insert item in queue to be extracted with specified delay.
//|
//|   param float delay: time after item comes due, in seconds.
//|   param object item: arbitrary object, typically a handler function or object.
//|
STATIC mp_obj_t mod_time_queue_after(const mp_obj_t self_in, const mp_obj_t time, const mp_obj_t callback) {
    mp_obj_time_queue_t *heap = get_heap(self_in);
    if (heap->len == heap->alloc) {
        mp_raise_IndexError(translate("queue overflow"));
    }
    mp_uint_t l = heap->len;
    heap->items[l].time = common_hal_time_monotonic() + (long)(1e3*mp_obj_get_float(time));
    heap->items[l].callback = callback;
    heap_siftdown(heap, 0, heap->len);
    heap->len++;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_time_queue_after_obj, mod_time_queue_after);

//|   .. method:: pop()
//|
//|     Remove top item from queue. Typically call peek_time first.
//|     Raises IndexError if queue is empty.
//|
//|   return object: item.
//|
STATIC mp_obj_t mod_time_queue_pop(mp_obj_t self_in) {
    mp_obj_time_queue_t *heap = get_heap(self_in);
    if (heap->len == 0) {
        mp_raise_IndexError(translate("heap empty"));
    }
    struct qentry *item = &heap->items[0];
    mp_obj_t result = item->callback;
    heap->len -= 1;
    heap->items[0] = heap->items[heap->len];
    heap->items[heap->len].callback = MP_OBJ_NULL; // so we don't retain a pointer
    if (heap->len) {
        heap_siftup(heap, 0);
    }
    return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_time_queue_pop_obj, mod_time_queue_pop);

//|   .. method:: get(index)
//|
//|     Item at specified index.
//|
//|   param int index: item index.
//|
//|   return object: item.
//|
STATIC mp_obj_t mod_time_queue_get(mp_obj_t self_in, mp_obj_t index_in) {
    mp_obj_time_queue_t *heap = get_heap(self_in);
    uint32_t index = mp_obj_get_int(index_in);
    if (index < 0 || index >= heap->len) {
        mp_raise_IndexError(NULL);
    }
    return heap->items[index].callback;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_time_queue_get_obj, mod_time_queue_get);

//|   .. method:: peek_time()
//|
//|     Time in seconds after which top item in queue comes due. Negative if item is past due.
//|
//|   return float: time.
//|
STATIC mp_obj_t mod_time_queue_peek_time(mp_obj_t self_in) {
    mp_obj_time_queue_t *heap = get_heap(self_in);
    if (heap->len == 0) {
        mp_raise_IndexError(translate("queue is empty"));
    }
    struct qentry *item = &heap->items[0];
    int64_t delta = item->time - common_hal_time_monotonic();
    return mp_obj_new_float(0.001*delta);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_time_queue_peek_time_obj, mod_time_queue_peek_time);

#if DEBUG
STATIC mp_obj_t mod_time_queue_dump(mp_obj_t self_in) {
    mp_obj_time_queue_t *heap = get_heap(self_in);
    for (int i = 0; i < heap->len; i++) {
        printf(UINT_FMT "\t%p\t%p\n", heap->items[i].time,
            MP_OBJ_TO_PTR(heap->items[i].callback), MP_OBJ_TO_PTR(heap->items[i].args));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_time_queue_dump_obj, mod_time_queue_dump);
#endif

STATIC mp_obj_t time_queue_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    mp_obj_time_queue_t *self = MP_OBJ_TO_PTR(self_in);
    switch (op) {
        case MP_UNARY_OP_BOOL: return mp_obj_new_bool(self->len != 0);
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(self->len);
        default: return MP_OBJ_NULL; // op not supported
    }
}

const mp_obj_property_t timer_time_queue_peek_time_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&mod_time_queue_peek_time_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC const mp_rom_map_elem_t time_queue_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_after), MP_ROM_PTR(&mod_time_queue_after_obj) },
    { MP_ROM_QSTR(MP_QSTR_pop), MP_ROM_PTR(&mod_time_queue_pop_obj) },
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mod_time_queue_get_obj) },
    // Property
    { MP_ROM_QSTR(MP_QSTR_peek_time), MP_ROM_PTR(&timer_time_queue_peek_time_obj) },

    #if DEBUG
    { MP_ROM_QSTR(MP_QSTR_dump), MP_ROM_PTR(&mod_time_queue_dump_obj) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(time_queue_locals_dict, time_queue_locals_dict_table);

const mp_obj_type_t timer_time_queue_type = {
    { &mp_type_type },
    .name = MP_QSTR_time_queue,
    .make_new = time_queue_make_new,
    .unary_op = time_queue_unary_op,
    .locals_dict = (void*)&time_queue_locals_dict,
};
