#pragma once

#include <stdio.h>

#include <tamtypes.h>

#include "sleep.h"
#include "errno.h"

#ifdef _EE
#define BOOT_ROM_ADDR (0x1FC00000 + 0x000000C0)
#else
#define BOOT_ROM_ADDR (0xBFC00000 + 0x000000C0)
#endif

#define BOOT_ROM_ADDR_VALUE 0x00000000 // value BOOT_ROM_ADDR points to

#define MODCHIP_CMD_RETRIES 3

inline static void modchip_poke_u8(u8 byte)
{
    *(volatile u8*)BOOT_ROM_ADDR = byte;
    // if writing "too fast", CE will stay high the entire time,
    // without WE wired up, modchip can only see a single byte,
    // force CE low by waiting for a bit
    sleep_us(4);
}

inline static void modchip_poke_u16(u16 v)
{
    modchip_poke_u8(((u8 *)&v)[0]);
    modchip_poke_u8(((u8 *)&v)[1]);
}

inline static void modchip_poke_u32(u32 v)
{
    modchip_poke_u8(((u8 *)&v)[0]);
    modchip_poke_u8(((u8 *)&v)[1]);
    modchip_poke_u8(((u8 *)&v)[2]);
    modchip_poke_u8(((u8 *)&v)[3]);
}

inline static u8 modchip_peek_u8()
{
    return *(volatile u8 *)BOOT_ROM_ADDR;
}

inline static u16 modchip_peek_u16()
{
    return *(volatile u16 *)BOOT_ROM_ADDR;
}

inline static u32 modchip_peek_u32()
{
    return *(volatile u32 *)BOOT_ROM_ADDR;
}

inline static void modchip_peek_n(void *dst, u32 size)
{
    for (uiptr i = 0; i < size - (size % 4); i += 4)
        *(u32 *)(dst + i) = modchip_peek_u32();

    for (uiptr i = size - (size % 4); i < size; i += 1)
        *(u8 *)(dst + i) = modchip_peek_u8();

}

inline static bool modchip_cmd(u32 cmd)
{
    modchip_poke_u32(cmd);
    return modchip_peek_u32() == MODCHIP_CMD_RESULT_OK;
}

inline static bool modchip_cmd_with_retry(u32 cmd, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_cmd(cmd);
        if (ret)
            break;
    }

    return ret;
}
