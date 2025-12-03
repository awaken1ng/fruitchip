
#include <pico/multicore.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <pico/stdio.h>

#include "colored_status_led.h"

#include <boot_rom/handler.h>
#include "reset.h"
#include "apps.h"
#include "settings.h"
#include "panic.h"
#include "version.h"

void __time_critical_func(main_core1)()
{
    const uint32_t hz = 240;

    // inline `set_sys_clock_khz(hz * KHZ, true)`, saves ~180 us
    static_assert(hz == 240);
    static_assert(XOSC_HZ == 12000000);
    static_assert(PLL_COMMON_REFDIV == 1);
    static_assert(PICO_PLL_VCO_MIN_FREQ_HZ == 750 * MHZ);
    static_assert(PICO_PLL_VCO_MAX_FREQ_HZ == 1600 * MHZ);
    uint vco = 1440000000, postdiv1 = 6, postdiv2 = 1;
    set_sys_clock_pll(vco, postdiv1, postdiv2);

    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, hz * MHZ, hz * MHZ);

    stdio_init_all();

    printf("fruitchip\n");
    printf("rev: %s\n", GIT_REV);
    printf("flash size: 0x%x\n", PICO_FLASH_SIZE_BYTES);

    boot_rom_data_out_init_dma();

    colored_status_led_init(RGB_PIO, RGB_SM);

    settings_init();

    if (!apps_partition_detect())
    {
        printf("apps partition header not found\n");
        colored_status_led_set_on_with_color(RGB_ERR_APPS);
    }
    else
    {
        colored_status_led_set_on_with_color(RGB_OK);
    }

    reset_init_irq();
}

int __time_critical_func(main)()
{
    multicore_reset_core1();
    multicore_launch_core1(main_core1);
    boot_rom_data_out_init();

    while (true)
    {
        if (!pio_sm_is_rx_fifo_empty(pio0, BOOT_ROM_READ_SNIFFER_SM))
            read_handler(pio_sm_get(pio0, BOOT_ROM_READ_SNIFFER_SM));

        if (!pio_sm_is_rx_fifo_empty(pio0, BOOT_ROM_WRITE_SNIFFER_SM))
            write_handler(pio_sm_get(pio0, BOOT_ROM_WRITE_SNIFFER_SM));
    }

    return 0;
}
