#pragma once

#include "modchip/io.h"
#include "modchip/cmd.h"

inline static bool modchip_git_rev(char *dst)
{
    modchip_poke_u32(MODCHIP_CMD_GIT_REV);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    modchip_peek_n(dst, 8);

    return true;
}
