/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Yihui Xiong
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
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/objarray.h"

#include "common-hal/matrix/Matrix.h"
#include "shared-bindings/matrix/Matrix.h"
#include "supervisor/port.h"

//| class Matrix:
//|     """keyboard matrix"""
//|
STATIC mp_obj_t matrix_matrix_make_new(const mp_obj_type_t *type,
        mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    matrix_matrix_obj_t *self = m_new_obj(matrix_matrix_obj_t);
    self->base.type = &matrix_matrix_type;
    self->head = 0;
    self->tail = 0;
    self->size = sizeof(self->queue) / sizeof(self->queue[0]);

    common_hal_matrix_matrix_init(self);

    return MP_OBJ_FROM_PTR(self);
}

//|     rows: int = ...
//|     """The row of the matrix"""
//|
STATIC mp_obj_t matrix_matrix_get_rows(mp_obj_t self_in) {
    return MP_OBJ_NEW_SMALL_INT(8);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_get_rows_obj, matrix_matrix_get_rows);

const mp_obj_property_t matrix_matrix_rows_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&matrix_matrix_get_rows_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|     cols: int = ...
//|     """The column of the matrix"""
//|
STATIC mp_obj_t matrix_matrix_get_cols(mp_obj_t self_in) {
    return MP_OBJ_NEW_SMALL_INT(8);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_get_cols_obj, matrix_matrix_get_cols);
const mp_obj_property_t matrix_matrix_cols_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&matrix_matrix_get_cols_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|     keys: int = ...
//|     """The number of the matrix keys"""
//|
STATIC mp_obj_t matrix_matrix_get_keys(mp_obj_t self_in) {
    return MP_OBJ_NEW_SMALL_INT(64);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_get_keys_obj, matrix_matrix_get_keys);
const mp_obj_property_t matrix_matrix_keys_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&matrix_matrix_get_keys_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

STATIC mp_obj_t matrix_matrix_scan(mp_obj_t self_in) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t n = common_hal_matrix_matrix_scan(self);
    return MP_OBJ_NEW_SMALL_INT(n);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_scan_obj, matrix_matrix_scan);


STATIC mp_obj_t matrix_matrix_wait(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int timeout = args[ARG_timeout].u_int;
    if (timeout <= 0) {
        return MP_OBJ_NEW_SMALL_INT(common_hal_matrix_matrix_scan(self));
    }

    return MP_OBJ_NEW_SMALL_INT(common_hal_matrix_matrix_wait(self, timeout));
}
MP_DEFINE_CONST_FUN_OBJ_KW(matrix_matrix_wait_obj, 1, matrix_matrix_wait);

STATIC mp_obj_t matrix_matrix_view(mp_obj_t self_in, mp_obj_t n) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t i = mp_obj_get_int(n);
    uint8_t value = self->queue[(self->tail + i) & MATRIX_QUEUE_MASK];
    return MP_OBJ_NEW_SMALL_INT(value);
}
MP_DEFINE_CONST_FUN_OBJ_2(matrix_matrix_view_obj, matrix_matrix_view);

STATIC mp_obj_t matrix_matrix_get_keydown_time(mp_obj_t self_in, mp_obj_t n) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t key = mp_obj_get_int(n);
    uint32_t value = self->t0[key];
    return mp_obj_new_int_from_uint(value);
}
MP_DEFINE_CONST_FUN_OBJ_2(matrix_matrix_get_keydown_time_obj, matrix_matrix_get_keydown_time);

STATIC mp_obj_t matrix_matrix_get_keyup_time(mp_obj_t self_in, mp_obj_t n) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t key = mp_obj_get_int(n);
    uint32_t value = self->t1[key];
    return mp_obj_new_int_from_uint(value);
}
MP_DEFINE_CONST_FUN_OBJ_2(matrix_matrix_get_keyup_time_obj, matrix_matrix_get_keyup_time);

STATIC mp_obj_t matrix_matrix_ms(mp_obj_t self_in, mp_obj_t tick) {
    int t = mp_obj_get_int(tick);
    t = t * 1000 / 1024;
    return mp_obj_new_int(t);
}
MP_DEFINE_CONST_FUN_OBJ_2(matrix_matrix_ms_obj, matrix_matrix_ms);

STATIC mp_obj_t matrix_matrix_time(mp_obj_t self_in) {
    uint32_t t = port_get_raw_ticks(NULL);
    return mp_obj_new_int_from_uint(t);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_time_obj, matrix_matrix_time);

STATIC mp_obj_t matrix_matrix_get(mp_obj_t self_in) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t value = self->queue[self->tail & MATRIX_QUEUE_MASK];
    self->tail += 1;
    return MP_OBJ_NEW_SMALL_INT(value);
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_get_obj, matrix_matrix_get);

STATIC mp_obj_t matrix_matrix_deinit(mp_obj_t self_in) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_matrix_matrix_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_deinit_obj, matrix_matrix_deinit);

STATIC mp_obj_t matrix_matrix_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete item
        // slice deletion
        return MP_OBJ_NULL; // op not supported
    } else {
        matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
        if (MP_OBJ_IS_TYPE(index_in, &mp_type_slice)) {
            mp_raise_NotImplementedError(translate("Slices not supported"));
        } else {
            // Single index rather than slice.
            size_t index = mp_get_index(self->base.type, (self->head - self->tail), index_in, false);
            if (value == MP_OBJ_SENTINEL) {
                // load
                uint8_t value_out = self->queue[(self->tail + index) & MATRIX_QUEUE_MASK];
                return MP_OBJ_NEW_SMALL_INT(value_out);
            } else {
                mp_raise_AttributeError(translate("Read-only"));
            }
        }
    }
}

STATIC mp_obj_t matrix_matrix_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
    matrix_matrix_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t len = self->head - self->tail;
    switch (op) {
        case MP_UNARY_OP_BOOL: return mp_obj_new_bool(len != 0);
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(len);
        default: return MP_OBJ_NULL; // op not supported
    }
}

STATIC const mp_rom_map_elem_t matrix_matrix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&matrix_matrix_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_wait), MP_ROM_PTR(&matrix_matrix_wait_obj) },
    { MP_ROM_QSTR(MP_QSTR_view), MP_ROM_PTR(&matrix_matrix_view_obj) },
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&matrix_matrix_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_keydown_time), MP_ROM_PTR(&matrix_matrix_get_keydown_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_keyup_time), MP_ROM_PTR(&matrix_matrix_get_keyup_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&matrix_matrix_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_ms), MP_ROM_PTR(&matrix_matrix_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&matrix_matrix_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_rows), MP_ROM_PTR(&matrix_matrix_rows_obj) },
    { MP_ROM_QSTR(MP_QSTR_cols), MP_ROM_PTR(&matrix_matrix_cols_obj) },
    { MP_ROM_QSTR(MP_QSTR_keys), MP_ROM_PTR(&matrix_matrix_keys_obj) },

};
STATIC MP_DEFINE_CONST_DICT(matrix_matrix_locals_dict, matrix_matrix_locals_dict_table);

const mp_obj_type_t matrix_matrix_type = {
    { &mp_type_type },
    .name = MP_QSTR_Matrix,
    .make_new = matrix_matrix_make_new,
    .subscr = matrix_matrix_subscr,
    .unary_op = matrix_matrix_unary_op,
    .locals_dict = (mp_obj_dict_t*)&matrix_matrix_locals_dict,
};
