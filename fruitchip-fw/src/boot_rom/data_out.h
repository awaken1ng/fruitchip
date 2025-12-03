#pragma once

#include <stdint.h>
#include <stdio.h>

#include "hardware/dma.h"
#include "hardware/pio.h"

#include "modchip/errno.h"

#include "boot_rom.pio.h"

#define BOOT_ROM_DATA_OUT_CTRL1_CHAN 0
#define BOOT_ROM_DATA_OUT_CTRL2_CHAN 1
#define BOOT_ROM_DATA_OUT_DATA1_CHAN 2
#define BOOT_ROM_DATA_OUT_DATA2_CHAN 3
#define BOOT_ROM_DATA_OUT_BUSY_PING 4
#define BOOT_ROM_DATA_OUT_BUSY_PONG 5

inline static void dma_channel_set_chain_to(uint channel, uint chain_to, bool trigger)
{
    assert(chain_to <= NUM_DMA_CHANNELS);

    if (!trigger)
        dma_channel_hw_addr(channel)->al1_ctrl = (dma_channel_hw_addr(channel)->al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
    else
        dma_channel_hw_addr(channel)->ctrl_trig = (dma_channel_hw_addr(channel)->al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

void boot_rom_data_out_init();

void boot_rom_data_out_init_dma();

inline static void boot_rom_data_out_start()
{
    pio_set_sm_mask_enabled(pio0, (1 << BOOT_ROM_DATA_OUT_SM), true);
}

inline static void boot_rom_data_out_stop()
{
    pio_set_sm_mask_enabled(pio0, (1 << BOOT_ROM_DATA_OUT_SM), false);
}

inline static void boot_rom_data_out_reset()
{
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_DATA_OUT_SM));
    pio_sm_exec(pio0, BOOT_ROM_DATA_OUT_SM, pio_encode_jmp(boot_rom_data_out_offset));
}

inline static void boot_rom_sniffers_start()
{
    pio_set_sm_mask_enabled(pio0, (1 << BOOT_ROM_READ_SNIFFER_SM) | (1 << BOOT_ROM_WRITE_SNIFFER_SM), true);
}

inline static void boot_rom_sniffers_stop()
{
    pio_set_sm_mask_enabled(pio0, (1 << BOOT_ROM_READ_SNIFFER_SM) | (1 << BOOT_ROM_WRITE_SNIFFER_SM), false);
}

inline static void boot_rom_sniffers_reset()
{
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_READ_SNIFFER_SM) | (1 << BOOT_ROM_WRITE_SNIFFER_SM));
    pio_sm_exec(pio0, BOOT_ROM_READ_SNIFFER_SM, pio_encode_jmp(boot_rom_read_sniffer_offset));
    pio_sm_exec(pio0, BOOT_ROM_WRITE_SNIFFER_SM, pio_encode_jmp(boot_rom_write_sniffer_offset));
}

void boot_rom_data_out_set_data(const volatile void *read_addr, uint32_t encoded_transfer_count);

void boot_rom_data_out_start_data_without_status_code(bool crc);

void boot_rom_data_out_start_data_with_status_code(uint32_t status_code, const volatile void *read_addr, uint32_t encoded_transfer_count, bool crc);

void boot_rom_data_out_start_status_code(uint32_t status_code);

void boot_rom_data_out_start_busy_code();

void boot_rom_data_out_stop_busy_code(uint32_t status_code);
