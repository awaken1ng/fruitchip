#pragma once

#include "modchip/io.h"
#include "modchip/cmd.h"
#include "crc32.h"

inline static bool modchip_get_flash_size(u32 *flash_size)
{
    modchip_poke_u32(MODCHIP_CMD_GET_FLASH_SIZE);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    *flash_size = modchip_peek_u32();

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((uint8_t *)flash_size, sizeof(*flash_size));
    return crc_expected == crc_actual;
}
