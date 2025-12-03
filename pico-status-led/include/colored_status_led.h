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

// PICO_CONFIG: PICO_COLORED_STATUS_LED_AVAILABLE, Indicate whether a colored status LED is available, type=bool, default=1 if PICO_DEFAULT_WS2812_PIN is defined; may be set by the user to 0 to not use the colored status LED even if available, group=pico_status_led
#ifndef PICO_COLORED_STATUS_LED_AVAILABLE
#ifdef PICO_DEFAULT_WS2812_PIN
#define PICO_COLORED_STATUS_LED_AVAILABLE 1
#else
#define PICO_COLORED_STATUS_LED_AVAILABLE 0
#endif
#endif

// PICO_CONFIG: PICO_COLORED_STATUS_LED_USES_WRGB, Indicate if the colored status LED supports WRGB, type=bool, default=0, group=pico_status_led
#ifndef PICO_COLORED_STATUS_LED_USES_WRGB
#define PICO_COLORED_STATUS_LED_USES_WRGB 0
#endif

/*! \brief Generate an RGB color value for /ref colored_status_led_set_on_with_color
 *  \ingroup pico_status_led
 */
#ifndef PICO_COLORED_STATUS_LED_COLOR_FROM_RGB
#define PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#endif

/*! \brief Generate an WRGB color value for \ref colored_status_led_set_on_with_color
 *  \ingroup pico_status_led
 *
 *  \note If your hardware does not support a white pixel, the white component is ignored
 */
#ifndef PICO_COLORED_STATUS_LED_COLOR_FROM_WRGB
#define PICO_COLORED_STATUS_LED_COLOR_FROM_WRGB(w, r, g, b) (((w) << 24) | ((r) << 16) | ((g) << 8) | (b))
#endif

// PICO_CONFIG: PICO_DEFAULT_COLORED_STATUS_LED_ON_COLOR, the default pixel color value of the colored status LED when it is on, type=int, group=pico_status_led
#ifndef PICO_DEFAULT_COLORED_STATUS_LED_ON_COLOR
#if PICO_COLORED_STATUS_LED_USES_WRGB
#define PICO_DEFAULT_COLORED_STATUS_LED_ON_COLOR PICO_COLORED_STATUS_LED_COLOR_FROM_WRGB(0xaa, 0, 0, 0)
#else
#define PICO_DEFAULT_COLORED_STATUS_LED_ON_COLOR PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0xaa, 0xaa, 0xaa)
#endif
#endif

bool colored_status_led_init(PIO pio, uint sm);

void colored_status_led_deinit(void);

/*! \brief Determine if the `colored_status_led_` APIs are supported (i.e. if there is a colored status LED, and its
 *         use isn't disabled via \ref PICO_COLORED_STATUS_LED_AVAILABLE being set to 0
 *  \ingroup pico_status_led
 * \return true if the colored status LED API is available and expected to produce visible results
 * \sa PICO_COLORED_STATUS_LED_AVAILABLE
 */
static inline bool colored_status_led_supported(void) {
    return PICO_COLORED_STATUS_LED_AVAILABLE;
}

/*! \brief Set the colored status LED on or off
 *  \ingroup pico_status_led
 *
 * \note If your hardware does not support a colored status LED (PICO_DEFAULT_WS2812_PIN), this function does nothing and returns false.
 *
 * \param led_on true to turn the colored LED on. Pass false to turn the colored LED off
 * \return true if the colored status LED could be set, otherwise false
 */
bool colored_status_led_set_state(bool led_on);

/*! \brief Get the state of the colored status LED
 *  \ingroup pico_status_led
 *
 * \note If your hardware does not support a colored status LED (PICO_DEFAULT_WS2812_PIN), this function returns false.
 *
 * \return true if the colored status LED is on, or false if the colored status LED is off
 */
bool colored_status_led_get_state(void);

/*! \brief Ensure the colored status LED is on, with the specified color
 *  \ingroup pico_status_led
 *
 * \note If your hardware does not support a colored status LED (PICO_DEFAULT_WS2812_PIN), this function does nothing and returns false.
 *
 * \param color The color to use for the colored status LED when it is on, in 0xWWRRGGBB format
 * \return true if the colored status LED could be set, otherwise false on failure
 */
bool colored_status_led_set_on_with_color(uint32_t color);

/*! \brief Get the color used for the status LED value when it is on
 *  \ingroup pico_status_led
 *
 * \note If your hardware does not support a colored status LED (PICO_DEFAULT_WS2812_PIN), this function always returns 0x0.
 *
* \return The color used for the colored status LED when it is on, in 0xWWRRGGBB format
*/
uint32_t colored_status_led_get_on_color(void);

#ifdef __cplusplus
}
#endif

#endif
