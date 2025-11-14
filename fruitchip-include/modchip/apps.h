#pragma once

#include <errno.h>

#include "io.h"
#include "cmd.h"
#include "../crc32.h"

#define MODCHIP_APPS_SIZE_OFFSET 0
#define MODCHIP_APPS_SIZE_SIZE sizeof(u32)

#define MODCHIP_APPS_ATTR_OFFSET 4
#define MODCHIP_APPS_ATTR_SIZE sizeof(u32)

#define MODCHIP_APPS_DATA_OFFSET 8

#define MODCHIP_APPS_CRC_OFFSET(size) (MODCHIP_APPS_DATA_OFFSET + size)
#define MODCHIP_APPS_CRC_SIZE sizeof(u32)

#define MODCHIP_APPS_ATTR_DISABLE_NEXT_OSDSYS_HOOK (1 << 0)

inline static s32 modchip_apps_read(u32 offset, u32 size, u8 app_idx, void *dst, bool with_crc)
{
    modchip_poke_u32(MODCHIP_CMD_READ_APP);
    modchip_poke_u32(offset);
    modchip_poke_u32(offset ^ 0xFFFFFFFF);
    modchip_poke_u32(size);
    modchip_poke_u32(size ^ 0xFFFFFFFF);
    modchip_poke_u8(app_idx);
    modchip_poke_u8(app_idx ^ 0xFF);
    modchip_poke_u8(with_crc);
    modchip_poke_u8(with_crc ^ 0xFF);

    int r = modchip_peek_u32();
    if (r != MODCHIP_CMD_RESULT_OK)
        return -EPROTO;

    for (uiptr i = 0; i < size; i += 4)
        *(u32 *)(dst + i) = modchip_peek_u32();

    for (uiptr i = size - (size % 4); i < size; i += 1)
        *(u8 *)(dst + i) = modchip_peek_u8();

    if (with_crc)
    {
        u32 crc_expected = modchip_peek_u32();
        u32 crc_actual = crc32((uint8_t *)dst, size);
        return crc_expected == crc_actual
            ? 0
            : -EIO;
    }
    else
    {
        return 0;
    }
}
