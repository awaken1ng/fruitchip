#include <hardware/gpio.h>
#include <hardware/timer.h>

#include "boot_rom_data_out.h"

static void __time_critical_func(reset_pressed)(uint gpio, uint32_t event_mask)
{
    static uint64_t last_reset_us = 0;

    (void)gpio;

    if (event_mask & GPIO_IRQ_EDGE_FALL)
    {
        uint64_t now_us = time_us_64();
        uint64_t diff_us = now_us - last_reset_us;

        if (diff_us > 10000) // 10 ms
        {
            // due to how data out works, there's a chance for console to be reset during data transfer,
            // which will result PS2 in reading wrong data and fail to boot,
            // and if data transfer was particularly large, console won't boot even after multiple resets,
            // as data out will still be active.
            boot_rom_data_out_stop();
            boot_rom_sniffers_reset();
            boot_rom_data_out_reset();

            last_reset_us = time_us_64();
        }
    }

    if (event_mask & GPIO_IRQ_EDGE_RISE)
    {
        last_reset_us = time_us_64();
    }
}

void reset_init_irq()
{
    gpio_init(RST_PIN);
    gpio_pull_up(RST_PIN);
    gpio_set_irq_enabled_with_callback(RST_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, reset_pressed);
}
