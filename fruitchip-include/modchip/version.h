#pragma once

#include "modchip/io.h"
#include "modchip/cmd.h"

inline static bool modchip_fw_git_rev(char *dst)
{
    modchip_poke_u32(MODCHIP_CMD_FW_GIT_REV);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    modchip_peek_n(dst, 8);

    return true;
}

inline static bool modchip_bootloader_git_rev(char *dst)
{
    modchip_poke_u32(MODCHIP_CMD_BOOTLOADER_GIT_REV);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    modchip_peek_n(dst, 8);

    return true;
}
