#pragma once

#include <hardware/pio.h>

#include <colored_status_led.h>

#define RGB_PIO pio0
#define RGB_SM 3

#define RGB_OK            PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x00, 0x10, 0x00) // green
#define RGB_ERR_APPS      PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x10, 0x00, 0x00) // red
#define RGB_ERR_SETTINGS  PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x00, 0x00, 0x10) // blue
#define RGB_OK_BOOTLOADER PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x10, 0x10, 0x00) // yellow
