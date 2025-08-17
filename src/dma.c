#include "dma.h"

inline static bool pio_sm_is_tx_fifo_stalled(PIO pio, uint sm)
{
    uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
    return (pio->fdebug & fdebug_tx_stall) != 0;
}

void __time_critical_func(byte_out_irq_handler)()
{
    pio_interrupt_clear(pio0, BYTE_OUT_IRQ);

    if (dma_channel_is_busy(TX_DATA_OUT_DMA_CHAN))
        return;

    if (!pio_sm_is_tx_fifo_empty(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    if (!pio_sm_is_tx_fifo_stalled(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    pio_interrupt_set(pio0, DATA_OUT_PAUSED_IRQ); // pause data out
    pio_interrupt_clear(pio0, DATA_IN_PAUSED_IRQ); // resume sniffer

    // reset data out
    pio_restart_sm_mask(pio0, (1 << BOOT_ROM_DATA_OUT_SM) | (1 << BOOT_ROM_DATA_PINDIRS_SWITCHER_SM));
    pio_sm_exec(pio0, BOOT_ROM_DATA_OUT_SM, pio_encode_jmp(boot_rom_data_out_offset));
    pio_sm_exec(pio0, BOOT_ROM_DATA_PINDIRS_SWITCHER_SM, pio_encode_jmp(boot_rom_data_pindirs_switcher_offset));
}
