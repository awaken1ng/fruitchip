#pragma once

#include <stdint.h>

#include "hardware/dma.h"
#include "hardware/pio.h"

#include "boot_rom.pio.h"

#define TX_DATA_OUT_DMA_CHAN 0

inline static void dma_data_out_init()
{
    dma_channel_config dma_tx_data_conf;

    // configure data out DMA
    dma_channel_claim(TX_DATA_OUT_DMA_CHAN);
    dma_tx_data_conf = dma_channel_get_default_config(TX_DATA_OUT_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_data_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_data_conf, true);
    channel_config_set_write_increment(&dma_tx_data_conf, false);
    channel_config_set_dreq(&dma_tx_data_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_write_addr(TX_DATA_OUT_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_config(TX_DATA_OUT_DMA_CHAN, &dma_tx_data_conf, false);
}

inline static void pio_interrupt_set(PIO pio, uint pio_interrupt_num)
{
    check_pio_param(pio);
    invalid_params_if(HARDWARE_PIO, pio_interrupt_num >= 8);
    pio->irq_force = (1u << pio_interrupt_num);
}

inline static void dma_data_out_start_transfer(const volatile void *read_addr, uint32_t encoded_transfer_count)
{
    pio_interrupt_set(pio0, DATA_IN_PAUSED_IRQ); // pause sniffer
    dma_channel_transfer_from_buffer_now(TX_DATA_OUT_DMA_CHAN, read_addr, encoded_transfer_count);
    pio_interrupt_clear(pio0, DATA_OUT_PAUSED_IRQ); // resume data out

    // reset sniffers
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_READ_SNIFFER_SM) | (1 << BOOT_ROM_WRITE_SNIFFER_SM));
    pio_sm_exec(pio0, BOOT_ROM_READ_SNIFFER_SM, pio_encode_jmp(boot_rom_read_sniffer_offset));
    pio_sm_exec(pio0, BOOT_ROM_WRITE_SNIFFER_SM, pio_encode_jmp(boot_rom_write_sniffer_offset));
}

void byte_out_irq_handler();
