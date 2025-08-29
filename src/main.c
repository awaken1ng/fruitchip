#include <stdio.h>

#include <hardware/clocks.h>
#include <pico/stdio_rtt.h>
#include <pico/status_led.h>

#include "boot_rom_read_handler.h"
#include "boot_rom_write_handler.h"
#include "reset.h"
#include "apps.h"

int __time_critical_func(main)()
{
    stdio_init_all();

    uint32_t hz = 240;
    set_sys_clock_khz(hz * KHZ, true);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, hz * MHZ, hz * MHZ);

    boot_rom_data_out_init(pio0);

    // status led claims unused SM for WS2812, call it after claiming SMs for data out
    status_led_init();

    if (!apps_partition_detect())
    {
        colored_status_led_set_on_with_color(PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0xaa, 0x00, 0x00)); // red
        panic("Apps partition header not found");
    }

    colored_status_led_set_on_with_color(PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x00, 0x40, 0x00)); // green

    reset_init_irq();

    printf("core0: entering loop\n");
    while (true)
    {
        if (!pio_sm_is_rx_fifo_empty(pio0, BOOT_ROM_READ_SNIFFER_SM))
            read_handler(pio_sm_get(pio0, BOOT_ROM_READ_SNIFFER_SM));

        if (!pio_sm_is_rx_fifo_empty(pio0, BOOT_ROM_WRITE_SNIFFER_SM))
            write_handler(pio_sm_get(pio0, BOOT_ROM_WRITE_SNIFFER_SM));
    }

    return 0;
}
