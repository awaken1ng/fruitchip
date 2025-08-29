#include <stdio.h>

#include "boot_rom_data_out.h"

inline static bool pio_sm_is_tx_fifo_stalled(PIO pio, uint sm)
{
    uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
    return (pio->fdebug & fdebug_tx_stall) != 0;
}

void __time_critical_func(boot_rom_byte_out_irq_handler)()
{
    pio_interrupt_clear(pio0, BOOT_ROM_BYTE_OUT_IRQ);

    if (dma_channel_is_busy(BOOT_ROM_DATA_OUT_DMA_CHAN))
        return;

    if (!pio_sm_is_tx_fifo_empty(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    if (!pio_sm_is_tx_fifo_stalled(pio0, BOOT_ROM_DATA_OUT_SM))
        return;

    boot_rom_data_out_stop();
    boot_rom_data_out_reset();
}
