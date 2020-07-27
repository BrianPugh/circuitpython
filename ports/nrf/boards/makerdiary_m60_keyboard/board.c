/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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

#include "boards/board.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "common-hal/microcontroller/Pin.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"
#include "nrfx/drivers/include/nrfx_power.h"

#define DFU_MAGIC_UF2_RESET             0x57

#define BUTTON      27
#define BAT_EN      28
#define LED_R       30
#define LED_G       29
#define LED_B       31
#define RGB_MATRIX_EN   36


APP_TIMER_DEF(m_timer_id);

digitalio_digitalinout_obj_t led_r;
digitalio_digitalinout_obj_t led_g;
digitalio_digitalinout_obj_t led_b;

void timeout_handler(void *p)
{
  if (!(NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk)) {
    nrf_gpio_pin_write(BAT_EN, 0);
    // nrf_gpio_pin_write(RGB_MATRIX_EN, 0);
    nrf_gpio_pin_write(LED_R, 1);
    while (!nrf_gpio_pin_read(BUTTON)) {
      NRFX_DELAY_US(1000);
    }
    NRFX_DELAY_US(100000);
  } else {
    NRF_POWER->GPREGRET = DFU_MAGIC_UF2_RESET;
  }

	NVIC_SystemReset();
}

void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  if (!nrf_gpio_pin_read(BUTTON)) {
    app_timer_start(m_timer_id, APP_TIMER_TICKS(3000), NULL);
    nrf_gpio_pin_write(LED_R, 0);
  } else {
    app_timer_stop(m_timer_id);
    nrf_gpio_pin_write(LED_R, 1);
  }
}

void board_init(void) {
  never_reset_pin_number(BUTTON);
  never_reset_pin_number(BAT_EN);
  // never_reset_pin_number(RGB_MATRIX_EN);

  // nrf_gpio_cfg_output(RGB_MATRIX_EN);
  // nrf_gpio_pin_write(RGB_MATRIX_EN, 0);

  common_hal_digitalio_digitalinout_construct(&led_r, &pin_P0_30);
  common_hal_digitalio_digitalinout_construct(&led_g, &pin_P0_29);
  common_hal_digitalio_digitalinout_construct(&led_b, &pin_P0_31);
  led_r.base.type = &digitalio_digitalinout_type;
  led_g.base.type = &digitalio_digitalinout_type;
  led_b.base.type = &digitalio_digitalinout_type;
  common_hal_digitalio_digitalinout_switch_to_output(&led_r, true, DRIVE_MODE_PUSH_PULL);
  common_hal_digitalio_digitalinout_switch_to_output(&led_g, true, DRIVE_MODE_PUSH_PULL);
  common_hal_digitalio_digitalinout_switch_to_output(&led_b, true, DRIVE_MODE_PUSH_PULL);
  
  common_hal_digitalio_digitalinout_never_reset(&led_r);
  common_hal_digitalio_digitalinout_never_reset(&led_g);
  common_hal_digitalio_digitalinout_never_reset(&led_b);

  // nrf_gpio_cfg_output(LED_R);
  // nrf_gpio_pin_write(LED_R, 1);

  // nrf_gpio_cfg_output(LED_G);
  // nrf_gpio_pin_write(LED_G, 0);

  // never_reset_pin_number(LED_R);
  // never_reset_pin_number(LED_G);
  // never_reset_pin_number(LED_B);

  app_timer_init();

  // app_timer_id_t m_timer_id;
  app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);

  if (nrfx_gpiote_is_init()) {
    nrfx_gpiote_uninit();
  }
  nrfx_gpiote_init(NRFX_GPIOTE_CONFIG_IRQ_PRIORITY);

  nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
  config.pull = NRF_GPIO_PIN_PULLUP;

  nrfx_gpiote_in_init(BUTTON, &config, button_event_handler);

  nrfx_gpiote_in_event_enable(BUTTON, true);
}

bool board_requests_safe_mode(void) {
  return false;
}

void reset_board(void) {

}
