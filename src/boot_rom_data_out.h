#pragma once

#include <stdint.h>

#include "hardware/dma.h"
#include "hardware/pio.h"

#include "boot_rom.pio.h"

#define BOOT_ROM_DATA_OUT_DMA_CHAN 0

inline static void pio_interrupt_set(PIO pio, uint pio_interrupt_num)
{
    check_pio_param(pio);
    invalid_params_if(HARDWARE_PIO, pio_interrupt_num >= 8);
    pio->irq_force = (1u << pio_interrupt_num);
}

void boot_rom_byte_out_irq_handler();

inline static void boot_rom_data_out_stop()
{
    pio_interrupt_set(pio0, BOOT_ROM_DATA_OUT_PAUSED_IRQ); // pause data out
    pio_interrupt_clear(pio0, BOOT_ROM_DATA_IN_PAUSED_IRQ); // resume sniffer
}

inline static void boot_rom_data_out_init(PIO pio)
{
    gpio_pull_up(BOOT_ROM_Q0_PIN + 0);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 1);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 2);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 3);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 4);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 5);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 6);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 7);
    gpio_pull_up(BOOT_ROM_CE_PIN);
    gpio_pull_up(BOOT_ROM_OE_PIN);

    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 0, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 1, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 2, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 3, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 4, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 5, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 6, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 7, GPIO_DRIVE_STRENGTH_12MA);

    // assign hardcoded SM indices to avoid runtime calculations of masks
    pio_sm_claim(pio, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_claim(pio, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_claim(pio, BOOT_ROM_DATA_OUT_SM);
    pio_sm_claim(pio, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM);

    // setup initial IRQ state, start with data out paused
    boot_rom_data_out_stop();

    // load programs in specific order to match the statically calculated offsets
    int offset = pio_add_program(pio, &boot_rom_read_sniffer_program);
    boot_rom_read_sniffer_init_and_start(pio, BOOT_ROM_READ_SNIFFER_SM, boot_rom_read_sniffer_offset);
    if (boot_rom_read_sniffer_offset != offset) panic("Read sniffer loaded at unexpected offset");

    offset = pio_add_program(pio, &boot_rom_write_sniffer_program);
    boot_rom_write_sniffer_init_and_start(pio, BOOT_ROM_WRITE_SNIFFER_SM, boot_rom_write_sniffer_offset);
    if (boot_rom_write_sniffer_offset != offset) panic("Write sniffer loaded at unexpected offset");

    offset = pio_add_program(pio, &boot_rom_data_out_program);
    boot_rom_data_out_init_and_start(pio, BOOT_ROM_DATA_OUT_SM, boot_rom_data_out_offset);
    if (boot_rom_data_out_offset != offset) panic("Data out loaded at unexpected offset");

    offset = pio_add_program(pio, &boot_rom_data_pindirs_switcher_program);
    boot_rom_data_pindirs_switcher_init_and_start(pio, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM, boot_rom_data_pindirs_switcher_offset);
    if (boot_rom_data_pindirs_switcher_offset != offset) panic("Pindirs switcher loaded at unexpected offset");

    dma_channel_config dma_tx_data_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_DMA_CHAN);
    dma_tx_data_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_data_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_data_conf, true);
    channel_config_set_write_increment(&dma_tx_data_conf, false);
    channel_config_set_dreq(&dma_tx_data_conf, pio_get_dreq(pio, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_DMA_CHAN, &pio->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_DMA_CHAN, &dma_tx_data_conf, false);

    pio_set_irq0_source_enabled(pio, pis_interrupt0 + BOOT_ROM_BYTE_OUT_IRQ, true);
    irq_set_exclusive_handler(PIO0_IRQ_0 + NUM_PIO_IRQS * PIO_NUM(pio), boot_rom_byte_out_irq_handler);
    irq_set_enabled(PIO0_IRQ_0 + NUM_PIO_IRQS * PIO_NUM(pio), true);
}

inline static void boot_rom_data_out_reset()
{
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_DATA_OUT_SM) | (1 << BOOT_ROM_DATA_PINDIRS_SWITCHER_SM));
    pio_sm_exec(pio0, BOOT_ROM_DATA_OUT_SM, pio_encode_jmp(boot_rom_data_out_offset));
    pio_sm_exec(pio0, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM, pio_encode_jmp(boot_rom_data_pindirs_switcher_offset));
}

inline static void boot_rom_sniffers_reset()
{
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_READ_SNIFFER_SM) | (1 << BOOT_ROM_WRITE_SNIFFER_SM));
    pio_sm_exec(pio0, BOOT_ROM_READ_SNIFFER_SM, pio_encode_jmp(boot_rom_read_sniffer_offset));
    pio_sm_exec(pio0, BOOT_ROM_WRITE_SNIFFER_SM, pio_encode_jmp(boot_rom_write_sniffer_offset));
}

inline static void boot_rom_data_out_start(const volatile void *read_addr, uint32_t encoded_transfer_count)
{
    pio_interrupt_set(pio0, BOOT_ROM_DATA_IN_PAUSED_IRQ); // pause sniffer
    dma_channel_transfer_from_buffer_now(BOOT_ROM_DATA_OUT_DMA_CHAN, read_addr, encoded_transfer_count);
    pio_interrupt_clear(pio0, BOOT_ROM_DATA_OUT_PAUSED_IRQ); // resume data out
    boot_rom_sniffers_reset();
}

