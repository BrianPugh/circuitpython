
#include <stdint.h>
#include "nrfx.h"
#include "nrfx_gpiote.h"
#include "supervisor/port.h"
#include "common-hal/matrix/Matrix.h"
#include "shared-bindings/time/__init__.h"
#include "nrf_qspi.h"

#define MATRIX_ROWS 8
#define MATRIX_COLS 8

static uint8_t row_io[MATRIX_ROWS] = {5, 6, 7, 8, 41, 40, 12, 11};
static uint8_t col_io[MATRIX_COLS] = {19, 20, 21, 22, 23, 24, 25, 26};

volatile uint32_t matrix_interrupt_status = 0;
static int matrix_is_inited = 0;

static uint32_t read_cols(void);
static void init_cols(void);
static void enable_interrupt(void);
static void disable_interrupt(void);
static void init_rows(void);
static void unselect_rows(void);
static void select_rows(void);
static void unselect_row(uint8_t row);
static void select_row(uint8_t row);
static void deinit_rows(void);
static void deinit_cols(void);

void matrix_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // NRF_P0->OUTSET = 1 << 29;
    matrix_interrupt_status = 1;
}

int common_hal_matrix_matrix_init(matrix_matrix_obj_t *self)
{
    if (matrix_is_inited)
    {
        deinit_rows();
        deinit_cols();
    }
    init_rows();
    init_cols();
    matrix_is_inited = 1;
    return 0;
}

int common_hal_matrix_matrix_deinit(matrix_matrix_obj_t *self)
{
    deinit_rows();
    deinit_cols();
    matrix_is_inited = 0;
    return 0;
}

uint32_t common_hal_matrix_matrix_scan(matrix_matrix_obj_t *self)
{
    uint32_t active = 0;
    uint32_t scan_time = port_get_raw_ticks(NULL);  // unit: 1 / 1024 of a second

    for (unsigned row = 0; row < sizeof(row_io); row++) {
        select_row(row);
        NRFX_DELAY_US(1);
        uint32_t cols_value = read_cols();
        unselect_row(row);
        uint32_t mask = cols_value ^ self->value[row];
        uint8_t col = 0;
        while (mask) {
            if (mask & 1) {
                uint8_t key = row * MATRIX_COLS + col;
                if (cols_value & (1 << col)) {
                    // key down
                    self->queue[self->head & MATRIX_QUEUE_MASK] = key;
                    self->head++;
                    self->t0[key] = scan_time;
                } else {
                    // key up
                    self->queue[self->head & MATRIX_QUEUE_MASK] = 0x80 | key;
                    self->head++;
                    self->t1[key] = scan_time;
                }
            }
            mask >>= 1;
            col++;
        }
        self->value[row] = cols_value;

        active |= cols_value;
    }

    self->active = active;

    return self->head - self->tail;
}


uint32_t common_hal_matrix_matrix_wait(matrix_matrix_obj_t *self, int timeout)
{
    uint64_t start_tick = port_get_raw_ticks(NULL);
    // Adjust the delay to ticks vs ms.
    int64_t remaining = timeout * 1024 / 1000;
    uint64_t end_tick = start_tick + timeout;
    uint32_t n = self->head - self->tail;
    if (self->active) {
        do {
            uint32_t result = common_hal_matrix_matrix_scan(self);
            if (result > n) {
                n = result;
                break;
            }
            matrix_interrupt_status = 0;
            // select_rows();
            for (unsigned row = 0; row < sizeof(row_io); row++) {
                if (!self->value[row]) {
                    select_row(row);
                }
            }
            enable_interrupt();
            uint32_t tick = remaining < 4 ? remaining : 4;
            port_interrupt_after_ticks(tick);
            port_sleep_until_interrupt();
            disable_interrupt();
            unselect_rows();
            remaining = end_tick - port_get_raw_ticks(NULL);
        } while (remaining > 1 || matrix_interrupt_status);
    } else {
        select_rows();
        uint32_t cols = read_cols();
        matrix_interrupt_status = 0;
        if (!cols) {
            enable_interrupt();
            do {
                port_interrupt_after_ticks(remaining);
                port_sleep_until_interrupt();
                if (matrix_interrupt_status) {
                    break;
                }
                remaining = end_tick - port_get_raw_ticks(NULL);
            } while (remaining > 1);
            disable_interrupt();
        }
        unselect_rows();

        if (cols || matrix_interrupt_status) {
            n = common_hal_matrix_matrix_scan(self);
        }
    }

    return n;
}

static void init_rows(void)
{
    for (int i = 0; i < MATRIX_ROWS; i++)
    {
        nrf_gpio_cfg_output(row_io[i]);
        nrf_gpio_pin_write(row_io[i], 1);
    }
}

static void init_cols(void)
{
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    config.pull = NRF_GPIO_PIN_PULLUP;

    if ( !nrfx_gpiote_is_init() ) {
        nrfx_gpiote_init(NRFX_GPIOTE_CONFIG_IRQ_PRIORITY);
    }

    for (int i = 0; i < MATRIX_COLS; i++)
    {
        nrfx_gpiote_in_init(col_io[i], &config, matrix_event_handler);
    }
}

static void deinit_rows(void)
{
    for (int i = 0; i < MATRIX_ROWS; i++)
    {
        nrf_gpio_cfg_default(row_io[i]);
    }
}

static void deinit_cols(void)
{
    for (int i = 0; i < MATRIX_COLS; i++)
    {
        nrfx_gpiote_in_uninit(col_io[i]);
    }
}

static void enable_interrupt(void)
{
    for (int i = 0; i < MATRIX_COLS; i++)
    {
        nrfx_gpiote_in_event_enable(col_io[i], true);
    }
}

static void disable_interrupt(void)
{
    for (int i = 0; i < MATRIX_COLS; i++)
    {
        nrfx_gpiote_in_event_disable(col_io[i]);
    }
}

/* Returns status of switches(1:on, 0:off) */
static uint32_t read_cols(void)
{
    uint32_t cols = 0;

    for (int i = 0; i < MATRIX_COLS; i++)
    {
        uint32_t value = nrf_gpio_pin_read(col_io[i]);
        value = value ? 0 : (1 << i);
        cols |= value;
    }
    return cols;
}

/*
 * Row pin configuration
 */
static void unselect_rows(void)
{
    for (int i = 0; i < MATRIX_ROWS; i++)
    {
        nrf_gpio_pin_write(row_io[i], 1);
    }
}

static void select_rows(void)
{
    for (int i = 0; i < MATRIX_ROWS; i++)
    {
        nrf_gpio_pin_write(row_io[i], 0);
    }
}

static void unselect_row(uint8_t row)
{
    nrf_gpio_pin_write(row_io[row], 1);
}

static void select_row(uint8_t row)
{
    nrf_gpio_pin_write(row_io[row], 0);
}
