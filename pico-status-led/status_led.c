/*
 * Copyright (c) 2025 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "status_led.h"
#include <stdio.h>

#if PICO_STATUS_LED_AVAILABLE && defined(PICO_DEFAULT_LED_PIN)
#define STATUS_LED_USING_GPIO 1
#else
#define STATUS_LED_USING_GPIO 0
#endif

bool status_led_init() {
    bool success = false;
    // ---- regular status LED ----
#if STATUS_LED_USING_GPIO
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    success = true;
#endif
    return success;
}

void status_led_deinit(void) {
#if STATUS_LED_USING_GPIO
    gpio_deinit(PICO_DEFAULT_LED_PIN);
#endif
}
