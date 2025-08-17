#include <stdio.h>

#include <hardware/clocks.h>
#include <pico/stdio_rtt.h>

#include "boot_rom.pio.h"
#include "boot_rom_read_handler.h"
#include "boot_rom_write_handler.h"
#include "dma.h"

int __time_critical_func(main)()
{
    stdio_init_all();

    uint32_t hz = 240;
    set_sys_clock_khz(hz * KHZ, true);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, hz * MHZ, hz * MHZ);

    gpio_pull_up(BOOT_ROM_Q0_PIN);
    gpio_pull_up(BOOT_ROM_Q1_PIN);
    gpio_pull_up(BOOT_ROM_Q2_PIN);
    gpio_pull_up(BOOT_ROM_Q3_PIN);
    gpio_pull_up(BOOT_ROM_Q4_PIN);
    gpio_pull_up(BOOT_ROM_Q5_PIN);
    gpio_pull_up(BOOT_ROM_Q6_PIN);
    gpio_pull_up(BOOT_ROM_Q7_PIN);
    gpio_pull_up(BOOT_ROM_CE_PIN);
    gpio_pull_up(BOOT_ROM_OE_PIN);

    gpio_set_drive_strength(BOOT_ROM_Q0_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q1_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q2_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q3_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q4_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q5_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q6_PIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q7_PIN, GPIO_DRIVE_STRENGTH_12MA);

    // claim and assign hardcoded SM indices to avoid runtime calculations of masks
    pio_sm_claim(pio0, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_claim(pio0, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_claim(pio0, BOOT_ROM_DATA_OUT_SM);
    pio_sm_claim(pio0, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM);

    // setup initial IRQ state, start with data out paused
    pio_interrupt_clear(pio0, DATA_IN_PAUSED_IRQ);
    pio_interrupt_set(pio0, DATA_OUT_PAUSED_IRQ);

    // load programs in specific order to match the statically calculated offsets
    int offset = pio_add_program(pio0, &boot_rom_read_sniffer_program);
    boot_rom_read_sniffer_init_and_start(pio0, BOOT_ROM_READ_SNIFFER_SM, boot_rom_read_sniffer_offset);
    if (boot_rom_read_sniffer_offset != offset) panic("Read sniffer loaded at unexpected offset");

    offset = pio_add_program(pio0, &boot_rom_write_sniffer_program);
    boot_rom_write_sniffer_init_and_start(pio0, BOOT_ROM_WRITE_SNIFFER_SM, boot_rom_write_sniffer_offset);
    if (boot_rom_write_sniffer_offset != offset) panic("Write sniffer loaded at unexpected offset");

    offset = pio_add_program(pio0, &boot_rom_data_out_program);
    boot_rom_data_out_init_and_start(pio0, BOOT_ROM_DATA_OUT_SM, boot_rom_data_out_offset);
    if (boot_rom_data_out_offset != offset) panic("Data out loaded at unexpected offset");

    offset = pio_add_program(pio0, &boot_rom_data_pindirs_switcher_program);
    boot_rom_data_pindirs_switcher_init_and_start(pio0, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM, boot_rom_data_pindirs_switcher_offset);
    if (boot_rom_data_pindirs_switcher_offset != offset) panic("Pindirs switcher loaded at unexpected offset");

    dma_data_out_init();

    pio_set_irq0_source_enabled(pio0, pis_interrupt0 + BYTE_OUT_IRQ, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, byte_out_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);

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
