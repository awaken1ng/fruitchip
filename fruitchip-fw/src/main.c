#include <stdio.h>

#include <hardware/clocks.h>
#include <pico/stdio_rtt.h>
#include <pico/status_led.h>

#include <boot_rom/handler.h>
#include "reset.h"
#include "apps.h"
#include "settings.h"
#include "panic.h"
#include "version.h"

int __time_critical_func(main)()
{
    stdio_init_all();

    uint32_t hz = 240;
    set_sys_clock_khz(hz * KHZ, true);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, hz * MHZ, hz * MHZ);

    printf("fruitchip\n");
    printf("rev: %s\n", GIT_REV);
    printf("flash size: 0x%x\n", PICO_FLASH_SIZE_BYTES);

    boot_rom_data_out_init();

    // status led claims unused SM for WS2812, call it after claiming SMs for data out
    status_led_init();

    if (!apps_partition_detect())
        panic_with_led(RGB_ERR_APPS, "Apps partition header not found");

    settings_init();

    colored_status_led_set_on_with_color(RGB_OK);

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
