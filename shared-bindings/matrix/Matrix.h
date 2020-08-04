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

#ifndef MICROPY_INCLUDED_SHARED_BINDINGS_MATRIX_MATRIX_H
#define MICROPY_INCLUDED_SHARED_BINDINGS_MATRIX_MATRIX_H

#include <stdint.h>
#include "py/obj.h"

#define MATRIX_QUEUE_SIZE   (1 << 6)
#define MATRIX_QUEUE_MASK   (MATRIX_QUEUE_SIZE - 1)

extern const mp_obj_type_t matrix_matrix_type;

typedef struct {
    mp_obj_base_t base;
    uint8_t queue[MATRIX_QUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t t;
    uint32_t t0[64];
    uint32_t t1[64];
    uint32_t raw[8];
    uint32_t value[8];
    uint32_t active;
    uint32_t debounce_ticks;
} matrix_matrix_obj_t;


#endif
