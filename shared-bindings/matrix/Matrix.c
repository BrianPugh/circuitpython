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

// #include "common-hal/matrix/Matrix.h"
#include "shared-bindings/matrix/Matrix.h"

//| class Matrix:
//|     """keyboard matrix"""
//|


STATIC mp_obj_t matrix_matrix_make_new(const mp_obj_type_t *type,
        mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    matrix_matrix_obj_t *self = m_new_obj(matrix_matrix_obj_t);
    self->base.type = &matrix_matrix_type;

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

STATIC mp_obj_t matrix_matrix_scan(mp_obj_t self_in) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_scan_obj, matrix_matrix_scan);


STATIC mp_obj_t matrix_matrix_wait(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    (void)n_args;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(matrix_matrix_wait_obj, 0, matrix_matrix_wait);

STATIC mp_obj_t matrix_matrix_preview(mp_obj_t self_in, mp_obj_t n) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(matrix_matrix_preview_obj, matrix_matrix_preview);

STATIC mp_obj_t matrix_matrix_get(mp_obj_t self_in) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(matrix_matrix_get_obj, matrix_matrix_get);


STATIC const mp_rom_map_elem_t matrix_matrix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&matrix_matrix_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_wait), MP_ROM_PTR(&matrix_matrix_wait_obj) },
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&matrix_matrix_get_obj) },

    { MP_ROM_QSTR(MP_QSTR_rows), MP_ROM_PTR(&matrix_matrix_rows_obj) },
    { MP_ROM_QSTR(MP_QSTR_cols), MP_ROM_PTR(&matrix_matrix_cols_obj) },

};
STATIC MP_DEFINE_CONST_DICT(matrix_matrix_locals_dict, matrix_matrix_locals_dict_table);

const mp_obj_type_t matrix_matrix_type = {
    { &mp_type_type },
    .name = MP_QSTR_Matrix,
    .make_new = matrix_matrix_make_new,
    .locals_dict = (mp_obj_dict_t*)&matrix_matrix_locals_dict,
};
