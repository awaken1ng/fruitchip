#pragma once

#include <tamtypes.h>

#include "modchip/cmd.h"
#include "modchip/io.h"
#include "crc32.h"

inline static bool modchip_settings_get(u16 key, u32 *value)
{
    modchip_poke_u32(MODCHIP_CMD_KV_GET);
    modchip_poke_u16(key);
    modchip_poke_u16(key ^ 0xFFFF);

    int r = modchip_peek_u32();
    if (r != MODCHIP_CMD_RESULT_OK)
        return false;

    *value = modchip_peek_u32();

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((uint8_t *)value, sizeof(*value));
    return crc_expected == crc_actual;
}

inline static bool modchip_settings_set(u16 key, u32 value)
{
    modchip_poke_u32(MODCHIP_CMD_KV_SET);
    modchip_poke_u16(key);
    modchip_poke_u16(key ^ 0xFFFF);
    modchip_poke_u32(value);
    modchip_poke_u32(value ^ 0xFFFFFFFF);

    while (true)
    {
        int r = modchip_peek_u32();
        switch (r)
        {
            case MODCHIP_CMD_RESULT_BUSY:
                continue;
            case MODCHIP_CMD_RESULT_OK:
                return true;
            case MODCHIP_CMD_RESULT_ERR:
            default:
                return false;
        }
    }
}
