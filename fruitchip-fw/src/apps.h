#pragma once

#include <stdint.h>
#include <string.h>

#include <hardware/regs/addressmap.h>

#include <boot_rom/data_out.h>

#define APPS_PARTITION_MAGIC1 "APPS"
#define APPS_PARTITION_MAGIC2 0x4FB80AB4
#define APPS_PARTITION_VERSION 2

#define APPS_PARTITION_ADDR (XIP_BASE + APPS_PARTITION_OFFSET)

inline static bool apps_partition_detect()
{
    bool is_magic1_valid = memcmp((void *)APPS_PARTITION_ADDR, APPS_PARTITION_MAGIC1, sizeof(uint32_t)) == 0;
    if (!is_magic1_valid) return false;

    bool is_magic2_valid = *(uint32_t *)(APPS_PARTITION_ADDR + 0x4) == APPS_PARTITION_MAGIC2;
    if (!is_magic2_valid) return false;

    bool is_version_valid = *(uint32_t *)(APPS_PARTITION_ADDR + 0x8) == APPS_PARTITION_VERSION;
    if (!is_version_valid) return false;

    return true;
}

inline static uint32_t apps_partition_entries_count()
{
    return *(uint32_t *)(APPS_PARTITION_ADDR + 0xC);
}

inline static uint8_t *apps_partition_entry_addr(uint32_t idx)
{
    uint32_t offset = *(uint32_t *)(APPS_PARTITION_ADDR + 0x10 + (idx * 2 * sizeof(uint32_t)));
    return (uint8_t *)(APPS_PARTITION_ADDR + offset);
}

inline static uint32_t apps_partition_entry_length(uint32_t idx)
{
    return *(uint32_t *)(APPS_PARTITION_ADDR + 0x14 + (idx * 2 * sizeof(uint32_t)));
}
