#include <stdio.h>

#include <hardware/clocks.h>
#include "hardware/dma.h"
#include "pico/stdio_rtt.h"
#include "pico/multicore.h"

#include "boot_rom_passive_reader.pio.h"
#include "utils.h"
#include "loader_ee.h"

#define BOOT_ROM_READ_SNIFFER_SM 0
#define BOOT_ROM_WRITE_SNIFFER_SM 1
#define BOOT_ROM_DATA_OUT_SM 2
#define BOOT_ROM_DATA_PINDIRS_SWITCHER_SM 3

#define TX_DATA_OUT_DMA_CHAN 0

static dma_channel_config dma_tx_data_conf;

static inline void start_data_out_transfer(const volatile void *read_addr, uint32_t encoded_transfer_count) {
    pio_interrupt_set(pio0, DATA_IN_PAUSED_IRQ); // pause sniffer
    dma_channel_transfer_from_buffer_now(TX_DATA_OUT_DMA_CHAN, read_addr, encoded_transfer_count);
    pio_interrupt_clear(pio0, DATA_OUT_PAUSED_IRQ); // resume data out
}

void handle_write_idle(uint8_t w);
void handle_write_payload_0_C0h_seen(uint8_t w);
void handle_write_payload_0_C1h_seen(uint8_t w);

void (*write_handler)(uint8_t) = &handle_write_idle;

void __time_critical_func(handle_write_idle)(uint8_t w) {
    switch (w) {
        case 0xC0: write_handler = &handle_write_payload_0_C0h_seen; break;
        case 0xC1: write_handler = &handle_write_payload_0_C1h_seen; break;
    }
}

void __time_critical_func(handle_write_payload_0_C0h_seen)(uint8_t w) {
    if (w == 0xCB) start_data_out_transfer(LOADER_EE_STAGE2, sizeof(LOADER_EE_STAGE2));
    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_payload_0_C1h_seen)(uint8_t w) {
    if (w == 0xCC) start_data_out_transfer(LOADER_EE_STAGE3, sizeof(LOADER_EE_STAGE3));
    write_handler = &handle_write_idle;
}

void __time_critical_func(core1_byte_out_irq_handler)() {
    pio_interrupt_clear(pio0, BYTE_OUT_IRQ);

    if (dma_channel_is_busy(TX_DATA_OUT_DMA_CHAN))
        return;

    if (!pio_sm_is_tx_fifo_empty(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    if (!pio_sm_is_tx_fifo_stalled(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    pio_interrupt_set(pio0, DATA_OUT_PAUSED_IRQ); // pause data out
    pio_interrupt_clear(pio0, DATA_IN_PAUSED_IRQ); // resume sniffer
}

void __time_critical_func(core1_main)() {
    pio_set_irq1_source_enabled(pio0, pis_interrupt0 + BYTE_OUT_IRQ, true);
    irq_set_exclusive_handler(PIO0_IRQ_1, core1_byte_out_irq_handler);
    irq_set_enabled(PIO0_IRQ_1, true);

    printf("core1: entering loop\n");
    while (true) {
        uint8_t w = pio_sm_get_blocking(pio0, BOOT_ROM_WRITE_SNIFFER_SM);
        write_handler(w);
        printf("w: %x\n", w);
    }
}

void handle_read_idle(uint8_t r);
void handle_read_osdsys_0_48h_seen(uint8_t r);
void handle_read_osdsys_1_8Ah_seen(uint8_t r);
void handle_read_osdsys_2_05h_seen(uint8_t r);
void handle_read_osdsys_3_00h_seen(uint8_t r);
void handle_read_osdsys_4_07h_seen(uint8_t r);
void handle_read_osdsys_5_found(uint8_t r);

static void (*read_handler)(uint8_t) = &handle_read_idle;

void __time_critical_func(handle_read_idle)(uint8_t r) {
    switch (r) {
        case 0x48: read_handler = &handle_read_osdsys_0_48h_seen; break;
    }
}

void __time_critical_func(handle_read_osdsys_0_48h_seen)(uint8_t r) {
    switch (r) {
        case 0x8A: read_handler = &handle_read_osdsys_1_8Ah_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_1_8Ah_seen)(uint8_t r) {
    switch (r) {
        case 0x05: read_handler = &handle_read_osdsys_2_05h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_2_05h_seen)(uint8_t r) {
    switch (r) {
        case 0x00: read_handler = &handle_read_osdsys_3_00h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_3_00h_seen)(uint8_t r) {
    switch (r) {
        case 0x48: read_handler = &handle_read_osdsys_0_48h_seen; break;
        case 0x07: read_handler = &handle_read_osdsys_4_07h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_4_07h_seen)(uint8_t r) {
    static uint16_t counter = 0;

    if (counter == 486 && r == 0x24) {
        // next byte is the injection window
        start_data_out_transfer(LOADER_EE_STAGE1, sizeof(LOADER_EE_STAGE1));
        read_handler = &handle_read_idle;
        counter = 0;
        return;
    } else if (counter > 486) {
        read_handler = &handle_read_idle;
        counter = 0;
        return;
    }

    counter += 1;
}

void __time_critical_func(core0_main_loop)() {
    while (true) {
        // byte is sampled at rising edge, there's ~200 ns until next rising edge
        uint8_t r = pio_sm_get_blocking(pio0, BOOT_ROM_READ_SNIFFER_SM);
        read_handler(r);
    }
}

int __time_critical_func(main)() {
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

    int boot_rom_read_sniffer_offset = pio_add_program(pio0, &boot_rom_read_sniffer_program);
    boot_rom_read_sniffer_init_and_start(pio0, BOOT_ROM_READ_SNIFFER_SM, boot_rom_read_sniffer_offset);

    int boot_rom_write_sniffer_offset = pio_add_program(pio0, &boot_rom_write_sniffer_program);
    boot_rom_write_sniffer_init_and_start(pio0, BOOT_ROM_WRITE_SNIFFER_SM, boot_rom_write_sniffer_offset);

    int boot_rom_data_out_offset = pio_add_program(pio0, &boot_rom_data_out_program);
    boot_rom_data_out_init_and_start(pio0, BOOT_ROM_DATA_OUT_SM, boot_rom_data_out_offset);

    int boot_rom_data_pindirs_switcher_offset = pio_add_program(pio0, &boot_rom_data_pindirs_switcher_program);
    boot_rom_data_pindirs_switcher_init_and_start(pio0, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM, boot_rom_data_pindirs_switcher_offset);

    // configure data out DMA
    dma_channel_claim(TX_DATA_OUT_DMA_CHAN);
    dma_tx_data_conf = dma_channel_get_default_config(TX_DATA_OUT_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_data_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_data_conf, true);
    channel_config_set_write_increment(&dma_tx_data_conf, false);
    channel_config_set_dreq(&dma_tx_data_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_write_addr(TX_DATA_OUT_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_config(TX_DATA_OUT_DMA_CHAN, &dma_tx_data_conf, false);

    printf("core0: starting core1\n");
    multicore_reset_core1();
    multicore_launch_core1(core1_main);

    printf("core0: entering loop\n");
    core0_main_loop();

    return 0;
}
