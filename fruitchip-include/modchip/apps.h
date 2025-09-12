#pragma once

#include "io.h"
#include "cmd.h"

#define MODCHIP_APPS_SIZE_OFFSET 0
#define MODCHIP_APPS_SIZE_SIZE sizeof(u32)

#define MODCHIP_APPS_ATTR_OFFSET 4
#define MODCHIP_APPS_ATTR_SIZE sizeof(u32)

#define MODCHIP_APPS_DATA_OFFSET 8

#define MODCHIP_APPS_CRC_OFFSET(size) (MODCHIP_APPS_DATA_OFFSET + size)
#define MODCHIP_APPS_CRC_SIZE sizeof(u32)

inline static bool modchip_apps_read(u32 offset, u32 size, u8 app_idx, void *dst)
{
    modchip_poke_u32(MODCHIP_CMD_READ_APP);
    modchip_poke_u32(offset);
    modchip_poke_u32(offset ^ 0xFFFFFFFF);
    modchip_poke_u32(size);
    modchip_poke_u32(size ^ 0xFFFFFFFF);
    modchip_poke_u8(app_idx);
    modchip_poke_u8(app_idx ^ 0xFF);

    int r = modchip_peek_u32();
    if (r != MODCHIP_CMD_RESULT_OK)
        return false;

    for (uiptr i = 0; i < size; i += 4)
        *(u32 *)(dst + i) = modchip_peek_u32();

    for (uiptr i = size - (size % 4); i < size; i += 1)
        *(u8 *)(dst + i) = modchip_peek_u8();

    return true;
}
