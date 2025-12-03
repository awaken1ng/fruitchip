#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <hardware/dma.h>
#include <hardware/watchdog.h>
#include <hardware/flash.h>
#include <hardware/sync.h>

#include "colored_status_led.h"

#include "modchip/update.h"
#include "version.h"
#include "led_color.h"

static_assert(FLASH_SECTOR_SIZE == 4096, "");

static int interrupts;

static int crc_channel;
static int memcpy_channel;

static uint8_t sector_buffer[FLASH_SECTOR_SIZE] __attribute__((aligned(FLASH_SECTOR_SIZE)));

void __time_critical_func(dma_init_crc)()
{
    dma_channel_config cfg;
    static uint8_t dummy;

    crc_channel = dma_claim_unused_channel(true);
    cfg = dma_channel_get_default_config(crc_channel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_sniff_enable(&cfg, true);
    dma_sniffer_enable(crc_channel, 0, false);

    dma_channel_set_write_addr(crc_channel, &dummy, false);
    dma_channel_set_config(crc_channel, &cfg, false);

    memcpy_channel = dma_claim_unused_channel(true);
    cfg = dma_channel_get_default_config(memcpy_channel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_ring(&cfg, true, 12); // log2(4096) == 12

    dma_channel_set_write_addr(memcpy_channel, &sector_buffer, false);
    dma_channel_set_transfer_count(memcpy_channel, FLASH_SECTOR_SIZE, false);
    dma_channel_set_config(memcpy_channel, &cfg, false);
}

void __time_critical_func(dma_init_memcpy)()
{
    dma_channel_config cfg;

    memcpy_channel = dma_claim_unused_channel(true);
    cfg = dma_channel_get_default_config(memcpy_channel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_ring(&cfg, true, 12); // log2(4096) == 12

    dma_channel_set_write_addr(memcpy_channel, &sector_buffer, false);
    dma_channel_set_transfer_count(memcpy_channel, FLASH_SECTOR_SIZE, false);
    dma_channel_set_config(memcpy_channel, &cfg, false);
}

static inline uint32_t bl2crc(uint32_t x)
{
    return *(uint32_t*)(x + 0xfc);
}

uint32_t __time_critical_func(dma_crc32)(const void *data, size_t len, uint32_t crc)
{
    dma_hw->sniff_data = crc;

    dma_channel_transfer_from_buffer_now(crc_channel, data, len);
    dma_channel_wait_for_finish_blocking(crc_channel);

    return dma_hw->sniff_data;
}

static bool __time_critical_func(is_fw_valid)()
{
    static uint32_t fw_partition_addr = XIP_BASE + FW_PARTITION_OFFSET;

    return dma_crc32((const void *)fw_partition_addr, 252, 0xffffffff) == bl2crc(fw_partition_addr);
}

static bool __time_critical_func(is_update_valid)()
{
    static uint32_t update_partition_addr = XIP_BASE + UPDATE_PARTITION_OFFSET;

    return dma_crc32((const void *)update_partition_addr, 252, 0xffffffff) == bl2crc(update_partition_addr);
}

void __time_critical_func(dma_memcpy_sector)(const void *src)
{
    dma_channel_set_read_addr(memcpy_channel, src, true);
    dma_channel_wait_for_finish_blocking(memcpy_channel);
}

void __time_critical_func(try_start_firmware)()
{
    uint32_t fw_partition_addr = XIP_BASE + FW_PARTITION_OFFSET;
    bool is_valid = is_fw_valid();

    if (!is_valid)
    {
        printf("fw bad\n");
    }
    else
    {
        printf("fw ok\n");

        static const uint32_t boot2_size = 0x100;
        register uint32_t code asm("r0") = fw_partition_addr + boot2_size;
        register uint32_t vtor asm("r1") = PPB_BASE + M0PLUS_VTOR_OFFSET;

        asm volatile(
            "str r0, [r1]\n"
            "ldmia r0, {r0, r1}\n"
            "msr msp, r0\n"
            "bx r1\n"
            :
            : "r" (code), "r" (vtor)
            :
        );

    };
}

void __time_critical_func(try_update_firmware)()
{
    uint32_t fw_partition_addr = XIP_BASE + FW_PARTITION_OFFSET;
    uint32_t update_partition_addr = XIP_BASE + UPDATE_PARTITION_OFFSET;

    bool is_update_ok = is_update_valid();

    if (!is_update_ok)
    {
        printf("update bad\n");
    }
    else
    {
        printf("update ok\n");

        dma_init_memcpy();

        // erase first fw sector, this "breaks" existing fw, and marks the update as to be in progress
        // in case update is interrupted, this should allow to retry the update
        printf("erasing first fw sector (0x%x)\n", fw_partition_addr);
        interrupts = save_and_disable_interrupts();
        flash_range_erase(FW_PARTITION_OFFSET, FLASH_SECTOR_SIZE);
        restore_interrupts(interrupts);

        // start copying from second sector
        printf("copying update from second sector\n");

        uint32_t copied_sectors = 0;
        uint32_t skipped_sectors = 0;
        for (uint32_t offset = FLASH_SECTOR_SIZE; offset < FW_PARTITION_SIZE_BYTES; offset += FLASH_SECTOR_SIZE)
        {
            const void *sector_addr = (const void *)(fw_partition_addr + offset);
            const void *update_addr = (const void *)(update_partition_addr + offset);

            // compare each sector, if update was interrupted, this should allow to skip already copied sectors
            bool is_copied = memcmp(sector_addr, update_addr, FLASH_SECTOR_SIZE) == 0;
            printf("sector addr 0x%x, is_copied %i\n", fw_partition_addr + offset, is_copied);

            if (!is_copied)
            {
                uint32_t fw_offset = FW_PARTITION_OFFSET + offset;

                dma_memcpy_sector(update_addr);
                interrupts = save_and_disable_interrupts();
                flash_range_erase(fw_offset, FLASH_SECTOR_SIZE);
                flash_range_program(fw_offset, sector_buffer, FLASH_SECTOR_SIZE);
                restore_interrupts(interrupts);
                copied_sectors += 1;
            }
            else
            {
                skipped_sectors += 1;
            }
        }

        // finish the update by writing the first sector
        printf("copying first sector 0x%x\n", FW_PARTITION_OFFSET);
        dma_memcpy_sector((const void *)update_partition_addr);
        interrupts = save_and_disable_interrupts();
        flash_range_program(FW_PARTITION_OFFSET, sector_buffer, FLASH_SECTOR_SIZE);
        restore_interrupts(interrupts);
        copied_sectors += 1;

        printf("update done\n");
        printf("copied %u sectors\n", copied_sectors);
        printf("skipped %u sectors\n", skipped_sectors);
    }
}

int __time_critical_func(main)()
{
    stdio_init_all();

    printf("fruitchip bootloader\n");
    printf("rev: %s\n", GIT_REV);

    colored_status_led_init(RGB_PIO, RGB_SM);
    colored_status_led_set_on_with_color(RGB_OK_BOOTLOADER);

    dma_init_crc();

    bool is_fw_ok = is_fw_valid();
    bool update_requested = watchdog_hw->scratch[0] == MODCHIP_UPDATE_MAGIC;
    bool should_update = update_requested || !is_fw_ok;
    if (should_update)
    {
        printf("update_requested %i, is_fw_ok %i\n", update_requested, is_fw_ok);
        try_update_firmware();
    }

    // if update was requested, avoid trigging it again
    watchdog_hw->scratch[0] = 0;

    try_start_firmware();

    // reset into built-in bootloader
    reset_usb_boot(0, 0);
}
