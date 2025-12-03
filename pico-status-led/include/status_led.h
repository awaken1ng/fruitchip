/*
 * Copyright (c) 2025 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** \file pico/status_led.h
 *  \defgroup pico_status_led pico_status_led
 *
 * \brief Enables access to the on-board status LED(s)
 *
 * Boards usually have access to one or two on-board status LEDs which are configured via the board header (PICO_DEFAULT_LED_PIN, or PICO_DEFAULT_WS2812_PIN).
 * This library hides the low-level details so you can use the status LEDs for all boards without changing your code.
 * \note If your board has both a single-color LED and a colored LED, you can independently control the single-color LED with the `status_led_` APIs, and the colored LED with the `colored_status_led_` APIs
 */

#ifndef _PICO_STATUS_LED_H
#define _PICO_STATUS_LED_H

#include "hardware/gpio.h"
#include <hardware/pio.h>

#ifdef __cplusplus
extern "C" {
#endif

// PICO_CONFIG: PICO_STATUS_LED_AVAILABLE, Indicate whether a single-color status LED is available, type=bool, default=1 if PICO_DEFAULT_LED_PIN is defined; may be set by the user to 0 to not use either even if they are available, group=pico_status_led
#ifndef PICO_STATUS_LED_AVAILABLE
#if defined(PICO_DEFAULT_LED_PIN)
#define PICO_STATUS_LED_AVAILABLE 1
#else
#define PICO_STATUS_LED_AVAILABLE 0
#endif
#endif

/*! \brief Initialize the status LED(s)
 * \ingroup pico_status_led
 *
 * Initialize the status LED(s) and the resources they need before use.
 *
 * \note You must call this function (or \ref status_led_init_with_context) before using any other pico_status_led functions.
 *
 * \return Returns true if the LED was initialized successfully, otherwise false on failure
 * \sa status_led_init_with_context
 */
bool status_led_init(void);

/*! \brief Determine if the single-color `status_led_` APIs are supported (i.e. if there is a regular LED, and its
 *         use isn't disabled via \ref PICO_STATUS_LED_AVAILABLE being set to 0, or if the colored status LED is being used for
 *         the single-color `status_led_` APIs
 *  \ingroup pico_status_led
 * \return true if the single-color status LED API is available and expected to produce visible results
 * \sa PICO_STATUS_LED_AVAILABLE
 */
static inline bool status_led_supported(void) {
    return PICO_STATUS_LED_AVAILABLE;
}

/*! \brief Set the status LED on or off
*  \ingroup pico_status_led
*
* \note If your hardware does not support a status LED, this function does nothing and returns false.
*
* \param led_on true to turn the LED on. Pass false to turn the LED off
* \return true if the status LED could be set, otherwise false
*/
static inline bool status_led_set_state(bool led_on) {
    if (status_led_supported()) {
#if defined(PICO_DEFAULT_LED_PIN)
    #if PICO_DEFAULT_LED_PIN_INVERTED
        gpio_put(PICO_DEFAULT_LED_PIN, !led_on);
    #else
        gpio_put(PICO_DEFAULT_LED_PIN, led_on);
    #endif
        return true;
#endif
    }
    return false;
}

/*! \brief Get the state of the status LED
 *  \ingroup pico_status_led
 *
 * \note If your hardware does not support a status LED, this function always returns false.
 *
 * \return true if the status LED is on, or false if the status LED is off
 */
static inline bool status_led_get_state() {
    if (status_led_supported()) {
#if defined(PICO_DEFAULT_LED_PIN)
    #if PICO_DEFAULT_LED_PIN_INVERTED
        return !gpio_get(PICO_DEFAULT_LED_PIN);
    #else
        return gpio_get(PICO_DEFAULT_LED_PIN);
    #endif
#endif
    }
    return false;
}

/*! \brief De-initialize the status LED(s)
 *  \ingroup pico_status_led
 *
 * De-initializes the status LED(s) when they are no longer needed.
 */
void status_led_deinit();

#ifdef __cplusplus
}
#endif

#endif
