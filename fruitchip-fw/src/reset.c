#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <hardware/watchdog.h>

static void __time_critical_func(reset_pressed)(uint gpio, uint32_t event_mask)
{
    static uint64_t last_reset_us = 0;

    (void)gpio;

    if (event_mask & GPIO_IRQ_EDGE_FALL)
    {
        last_reset_us = time_us_64();
    }

    if (event_mask & GPIO_IRQ_EDGE_RISE)
    {
        uint64_t now_us = time_us_64();
        uint64_t diff_us = now_us - last_reset_us;

        if (diff_us > 10000) // 10 ms
        {
            watchdog_reboot(0, 0, 0);
            last_reset_us = time_us_64();
        }
    }
}

void reset_init_irq()
{
    gpio_init(RST_PIN);
    gpio_pull_up(RST_PIN);
    gpio_set_irq_enabled_with_callback(RST_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, reset_pressed);
}
