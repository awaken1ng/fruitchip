#include "colored_status_led.h"

#if PICO_COLORED_STATUS_LED_AVAILABLE && defined(PICO_DEFAULT_WS2812_PIN)
#define COLORED_STATUS_LED_USING_WS2812_PIO 1
#else
#define COLORED_STATUS_LED_USING_WS2812_PIO 0
#endif

static uint32_t colored_status_led_on_color = PICO_DEFAULT_COLORED_STATUS_LED_ON_COLOR;
static bool colored_status_led_on;

#if COLORED_STATUS_LED_USING_WS2812_PIO
#include <hardware/pio.h>
#include "ws2812.pio.h"

// PICO_CONFIG: PICO_COLORED_STATUS_LED_WS2812_FREQ, Frequency per bit for the WS2812 colored status LED, type=int, default=800000, group=pico_status_led
#ifndef PICO_COLORED_STATUS_LED_WS2812_FREQ
#define PICO_COLORED_STATUS_LED_WS2812_FREQ 800000
#endif

static PIO _pio;
static uint _sm;
static uint offset;

// Extract from 0xWWRRGGBB
#define RED(c) (((c) >> 16) & 0xff)
#define GREEN(c) (((c) >> 8) & 0xff)
#define BLUE(c) (((c) >> 0) & 0xff)
#define WHITE(c) (((c) >> 24) && 0xff)

static bool set_ws2812(uint32_t value) {
    if (_pio) {
#if PICO_COLORED_STATUS_LED_USES_WRGB
        // Convert to 0xWWGGRRBB
        pio_sm_put_blocking(_pio, _sm, WHITE(value) << 24 | GREEN(value) << 16 | RED(value) << 8 | BLUE(value));
#else
        // Convert to 0xGGRRBB00
        pio_sm_put_blocking(_pio, _sm, GREEN(value) << 24 | RED(value) << 16 | BLUE(value) << 8);
#endif
        return true;
    }
    return false;
}
#endif

bool colored_status_led_set_on_with_color(uint32_t color) {
    colored_status_led_on_color = color;
    return colored_status_led_set_state(true);
}

uint32_t colored_status_led_get_on_color(void) {
    return colored_status_led_on_color;
}

bool colored_status_led_set_state(bool led_on) {
    bool success = false;
    if (colored_status_led_supported()) {
#if COLORED_STATUS_LED_USING_WS2812_PIO
        success = true;
        if (led_on) {
            // Turn the LED "on" even if it was already on, as the color might have changed
            success = set_ws2812(colored_status_led_on_color);
        } else if (!led_on && colored_status_led_on) {
            success = set_ws2812(0);
        }
#endif
    }
    if (success) colored_status_led_on = led_on;
    return success;
}

bool colored_status_led_get_state(void) {
    return colored_status_led_on;
}

bool colored_status_led_init(PIO pio, uint sm) {
    bool success = false;

#if COLORED_STATUS_LED_USING_WS2812_PIO
    if (!pio_sm_is_claimed(pio, sm)) {
        pio_sm_claim(_pio, _sm);
        _pio = pio;
        _sm = sm;

        offset = pio_add_program(_pio, &ws2812_program);
        ws2812_program_init(_pio, _sm, offset, PICO_DEFAULT_WS2812_PIN, PICO_COLORED_STATUS_LED_WS2812_FREQ, PICO_COLORED_STATUS_LED_USES_WRGB);
    #ifdef PICO_DEFAULT_WS2812_POWER_PIN
        gpio_init(PICO_DEFAULT_WS2812_POWER_PIN);
        gpio_set_dir(PICO_DEFAULT_WS2812_POWER_PIN, GPIO_OUT);
        gpio_put(PICO_DEFAULT_WS2812_POWER_PIN, true);
    #endif
    }

    success = true;
#endif
    return success;
}

void colored_status_led_deinit(void) {
#if COLORED_STATUS_LED_USING_WS2812_PIO
    if (_pio) {
        pio_remove_program_and_unclaim_sm(&ws2812_program, _pio, _sm, offset);
        _pio = NULL;
    }
#ifdef PICO_DEFAULT_WS2812_POWER_PIN
    gpio_put(PICO_DEFAULT_WS2812_POWER_PIN, false);
    gpio_deinit(PICO_DEFAULT_WS2812_POWER_PIN);
#endif
#endif
}
