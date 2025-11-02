#pragma once

#include <stdint.h>
#include <stdio.h>

#include "hardware/dma.h"
#include "hardware/pio.h"

#include "boot_rom.pio.h"
#include "modchip/errno.h"

#define BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN 0
#define BOOT_ROM_DATA_OUT_DATA_DMA_CHAN 1
#define BOOT_ROM_DATA_OUT_CRC_DMA_CHAN 2
#define BOOT_ROM_DATA_OUT_NULL_DMA_CHAN 3
#define BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN 4
#define BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN 5

extern uint32_t _boot_rom_data_out_status_code;
extern uint32_t _boot_rom_data_out_busy_code;

inline static void dma_channel_set_chain_to(uint channel, uint chain_to, bool trigger)
{
    assert(chain_to <= NUM_DMA_CHANNELS);

    if (!trigger)
        dma_channel_hw_addr(channel)->al1_ctrl = (dma_channel_hw_addr(channel)->al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
    else
        dma_channel_hw_addr(channel)->ctrl_trig = (dma_channel_hw_addr(channel)->al1_ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chain_to << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
}

void boot_rom_byte_out_irq_handler();

inline static void boot_rom_data_out_init()
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
    pio_sm_claim(pio0, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_claim(pio0, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_claim(pio0, BOOT_ROM_DATA_OUT_SM);

    // load programs in specific order to match the statically calculated offsets
    int offset = pio_add_program(pio0, &boot_rom_read_sniffer_program);
    boot_rom_read_sniffer_sm_init_and_start(pio0, BOOT_ROM_READ_SNIFFER_SM, boot_rom_read_sniffer_offset);
    if (boot_rom_read_sniffer_offset != offset) panic("Read sniffer loaded at unexpected offset");

    offset = pio_add_program(pio0, &boot_rom_write_sniffer_program);
    boot_rom_write_sniffer_sm_init_and_start(pio0, BOOT_ROM_WRITE_SNIFFER_SM, boot_rom_write_sniffer_offset);
    if (boot_rom_write_sniffer_offset != offset) panic("Write sniffer loaded at unexpected offset");

    offset = pio_add_program(pio0, &boot_rom_data_out_program);
    boot_rom_data_out_sm_init(pio0, BOOT_ROM_DATA_OUT_SM, boot_rom_data_out_offset);
    if (boot_rom_data_out_offset != offset) panic("Data out loaded at unexpected offset");

    dma_channel_config dma_tx_status_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN);
    dma_tx_status_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_status_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_status_conf, true);
    channel_config_set_write_increment(&dma_tx_status_conf, false);
    channel_config_set_dreq(&dma_tx_status_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &_boot_rom_data_out_status_code, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, sizeof(_boot_rom_data_out_status_code), false);

    dma_channel_config dma_tx_data_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN);
    dma_tx_data_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_data_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_data_conf, true);
    channel_config_set_write_increment(&dma_tx_data_conf, false);
    channel_config_set_dreq(&dma_tx_data_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    channel_config_set_chain_to(&dma_tx_data_conf, BOOT_ROM_DATA_OUT_CRC_DMA_CHAN);
    channel_config_set_sniff_enable(&dma_tx_data_conf, true);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_sniffer_enable(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, 0, false);

    dma_channel_config dma_tx_crc_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN);
    dma_tx_crc_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_crc_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_crc_conf, true);
    channel_config_set_write_increment(&dma_tx_crc_conf, false);
    channel_config_set_dreq(&dma_tx_crc_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    channel_config_set_chain_to(&dma_tx_crc_conf, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN);
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, &dma_hw->sniff_data, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, sizeof(dma_hw->sniff_data), false);

    // pad the data with a dummy byte.
    // checking if TX FIFO is empty doesn't tell if data is on the bus or not,
    // by adding one more byte to the transfer, when this byte is pulled from FIFO
    // we know that the last byte of data was on the bus and it's safe to stop data out
    static uint8_t _zero = 0;
    dma_channel_config dma_tx_null_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN);
    dma_tx_null_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_null_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_null_conf, false);
    channel_config_set_write_increment(&dma_tx_null_conf, false);
    channel_config_set_dreq(&dma_tx_null_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    channel_config_set_chain_to(&dma_tx_null_conf, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN); // no chain
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, &_zero, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, sizeof(_zero), false);

    dma_channel_config dma_tx_busy_conf;
    _boot_rom_data_out_busy_code = MODCHIP_CMD_RESULT_BUSY;
    dma_channel_claim(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN);
    dma_tx_busy_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_busy_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_busy_conf, true);
    channel_config_set_write_increment(&dma_tx_busy_conf, false);
    channel_config_set_dreq(&dma_tx_busy_conf, pio_get_dreq(pio0, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, &_boot_rom_data_out_busy_code, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, &pio0->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, sizeof(_boot_rom_data_out_busy_code), false);

    static uint32_t *_boot_rom_data_out_busy_code_ptr = &_boot_rom_data_out_busy_code;
    dma_channel_config dma_tx_busy_reset_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN);
    dma_tx_busy_reset_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN);
    channel_config_set_transfer_data_size(&dma_tx_busy_reset_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_tx_busy_reset_conf, false);
    channel_config_set_write_increment(&dma_tx_busy_reset_conf, false);
    channel_config_set_dreq(&dma_tx_busy_reset_conf, DREQ_FORCE);
    channel_config_set_chain_to(&dma_tx_busy_reset_conf, BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN);
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, &_boot_rom_data_out_busy_code_ptr, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, &dma_channel_hw_addr(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN)->read_addr, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, 1, false);

    dma_channel_set_config(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &dma_tx_status_conf, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, &dma_tx_data_conf, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, &dma_tx_crc_conf, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, &dma_tx_null_conf, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, &dma_tx_busy_conf, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, &dma_tx_busy_reset_conf, false);

    pio_set_irq0_source_enabled(pio0, pis_interrupt0 + BOOT_ROM_BYTE_OUT_IRQ, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, boot_rom_byte_out_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

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

inline static void boot_rom_data_out_start_data_with_status_code(uint32_t status_code, const volatile void *read_addr, uint32_t encoded_transfer_count, bool crc)
{
    // restore FIFOs join if it was unjoined by busy code
    // note, this also clears FIFOs, see `pio_sm_clear_fifos`
    hw_set_bits(&pio0->sm[BOOT_ROM_DATA_OUT_SM].shiftctrl, PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS);

    boot_rom_sniffers_stop();

    if (crc)
    {
        dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, &dma_hw->sniff_data, false);
        dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, sizeof(dma_hw->sniff_data), false);
        dma_sniffer_enable(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, 0, false);
        dma_sniffer_set_data_accumulator(0xFFFFFFFF);
        dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, false); // status -> data -> crc -> null
    }
    else
    {
        dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, 0, false);
        dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, false); // status -> data -> null
    }

    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, read_addr, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, encoded_transfer_count, false);

    _boot_rom_data_out_status_code = status_code;
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, false);
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &_boot_rom_data_out_status_code, true);

    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

inline static void boot_rom_data_out_start_data_without_status_code(const volatile void *read_addr, uint32_t encoded_transfer_count, bool crc)
{
    // restore FIFOs join if it was unjoined by busy code
    // note, this also clears FIFOs, see `pio_sm_clear_fifos`
    hw_set_bits(&pio0->sm[BOOT_ROM_DATA_OUT_SM].shiftctrl, PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS);

    if (crc)
    {
        dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, &dma_hw->sniff_data, false);
        dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, sizeof(dma_hw->sniff_data), false);
        dma_sniffer_enable(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, 0, false);
        dma_sniffer_set_data_accumulator(0xFFFFFFFF);
        dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, false); // data -> crc -> null
    }
    else
    {
        dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, 0, false);
        dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, false); // data -> null
    }

    boot_rom_sniffers_stop();
    dma_channel_transfer_from_buffer_now(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, read_addr, encoded_transfer_count);
    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

inline static void boot_rom_data_out_start_status_code(uint32_t status_code)
{
    // restore FIFOs join if it was unjoined by busy code
    // note, this also clears FIFOs, see `pio_sm_clear_fifos`
    hw_set_bits(&pio0->sm[BOOT_ROM_DATA_OUT_SM].shiftctrl, PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS);

    boot_rom_sniffers_stop();

    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CRC_DMA_CHAN, 0, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_DATA_DMA_CHAN, 0, false);

    _boot_rom_data_out_status_code = status_code;
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, false); // status -> null
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &_boot_rom_data_out_status_code, true);

    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

inline static void boot_rom_data_out_start_busy_code()
{
    // unjoin FIFOs to reduce queue depth, changes the minimum amount of busy codes from 3 to 2
    // note, this also clears FIFOs, see `pio_sm_clear_fifos`
    hw_clear_bits(&pio0->sm[BOOT_ROM_DATA_OUT_SM].shiftctrl, PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS);

    boot_rom_sniffers_stop();

    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, false); // busy <- reset
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_DMA_CHAN, BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, true); // busy -> reset
    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

inline static void boot_rom_data_out_stop_busy_code(uint32_t status_code)
{
    _boot_rom_data_out_status_code = status_code;
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, &_boot_rom_data_out_status_code, false);

    // busy and busy reset are running in ping-pong, chain busy reset -> status -> null to end data out
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, BOOT_ROM_DATA_OUT_NULL_DMA_CHAN, false);
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_RESET_DMA_CHAN, BOOT_ROM_DATA_OUT_STATUS_DMA_CHAN, false);
}
