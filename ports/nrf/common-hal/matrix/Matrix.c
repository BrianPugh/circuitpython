
#include <stdint.h>
#include "nrfx.h"
#include "nrfx_gpiote.h"
#include "supervisor/port.h"
#include "common-hal/matrix/Matrix.h"

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
    uint32_t scan_time = port_get_raw_ticks(NULL);  // unit: 1 / 1024 of a second
    uint8_t rows = 8;

    for (uint8_t row = 0; row < rows; row++) {
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
    }

    return self->head - self->tail;
}


uint32_t common_hal_matrix_matrix_wait(matrix_matrix_obj_t *self)
{
    select_rows();
    matrix_interrupt_status = 0;
    enable_interrupt();
    port_sleep_until_interrupt();
    disable_interrupt();
    unselect_rows();
    if (matrix_interrupt_status) {
        return common_hal_matrix_matrix_scan(self);
    }

    return self->head - self->tail;
}

// void common_hal_matrix_matrix_enter_interrupt_mode(void)
// {
//     if (!(NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk))
//     {
//         // Disable QSPI
//         // csn-pins = <45> - keep CS high when QSPI is diabled
//         NRF_P1->OUTSET = 1 << 13;
//         NRF_P1->PIN_CNF[13] = 3;

//         // workaround
//         *(volatile uint32_t *)0x40029010 = 1;
//         *(volatile uint32_t *)0x40029054 = 1;
//         NRF_QSPI->ENABLE = 0;
//     }
// }

// void common_hal_matrix_matrix_leave_interrupt_mode(void)
// {
//     if (!NRF_QSPI->ENABLE)
//     {
//         NRF_QSPI->ENABLE = 1;
//     }
// }

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
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
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
        nrfx_gpiote_in_event_enable(col_io[i], false);
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
