#include <hardware/gpio.h>
#include <hardware/timer.h>

#include <colored_status_led.h>

#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>
#include <boot_rom/read/idle.h>
#include <boot_rom/write/idle.h>
#include "boot_rom/write/disable_next_osdsys_hook.h"
#include "boot_rom/write/flash.h"

static void reset()
{
    // due to how data out works, there's a chance for console to be reset during data transfer,
    // which will result PS2 in reading wrong data and fail to boot,
    // and if data transfer was particularly large, console won't boot even after multiple resets,
    // as data out will still be active.

    boot_rom_sniffers_stop();
    boot_rom_data_out_stop();

    boot_rom_data_out_reset();
    boot_rom_sniffers_reset();

    dma_hw->abort = 1 << BOOT_ROM_DATA_OUT_CTRL1_CHAN |
                    1 << BOOT_ROM_DATA_OUT_CTRL2_CHAN |
                    1 << BOOT_ROM_DATA_OUT_DATA1_CHAN |
                    1 << BOOT_ROM_DATA_OUT_DATA2_CHAN |
                    1 << BOOT_ROM_DATA_OUT_BUSY_PING |
                    1 << BOOT_ROM_DATA_OUT_BUSY_PONG;

    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_CTRL1_CHAN);
    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_CTRL2_CHAN);
    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_DATA1_CHAN);
    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_DATA2_CHAN);
    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_BUSY_PING);
    dma_channel_wait_for_finish_blocking(BOOT_ROM_DATA_OUT_BUSY_PONG);

    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, &dma_hw->ch[BOOT_ROM_DATA_OUT_DATA1_CHAN].read_addr, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_CTRL2_CHAN, &dma_hw->ch[BOOT_ROM_DATA_OUT_DATA2_CHAN].read_addr, false);

    pio_sm_clear_fifos(pio0, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_clear_fifos(pio0, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_clear_fifos(pio0, BOOT_ROM_DATA_OUT_SM);

    pio_sm_drain_tx_fifo(pio0, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_drain_tx_fifo(pio0, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_drain_tx_fifo(pio0, BOOT_ROM_DATA_OUT_SM);

    disable_next_osdsys_hook = false;
    flash_write_lock = true;
    read_handler = handle_read_idle;
    write_handler = handle_write_idle;

    boot_rom_sniffers_start();
}

static void __time_critical_func(reset_pressed)(uint gpio, uint32_t event_mask)
{
    static uint64_t last_reset_us = 0;

    (void)gpio;

    if (event_mask & GPIO_IRQ_EDGE_FALL)
    {
        uint64_t now_us = time_us_64();
        uint64_t diff_us = now_us - last_reset_us;

        if (diff_us > 10000) // 10 ms
        {
            reset();

            last_reset_us = time_us_64();
        }
    }

    if (event_mask & GPIO_IRQ_EDGE_RISE)
    {
        last_reset_us = time_us_64();
    }
}

void reset_init_irq()
{
    gpio_init(RST_PIN);
    gpio_pull_up(RST_PIN);
    gpio_set_irq_enabled_with_callback(RST_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, reset_pressed);
}
