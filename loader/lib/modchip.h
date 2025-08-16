#ifndef __MODCHIP_H__
#define __MODCHIP_H__

#include <tamtypes.h>

#include "usleep.h"

#define BOOT_ROM_ADDR (0x1FC00000 + 0x000000C0) // points to 0x00000000

inline static void modchip_poke(u8 byte)
{
    *(volatile u8*)BOOT_ROM_ADDR = byte;
    // if writing "too fast", CE will stay high the entire time,
    // without WE wired up, modchip can only see a single byte,
    // force CE low by waiting for a bit
    usleep(2);
}

inline static u32 modchip_peek()
{
    u32 v = *(volatile u32*)BOOT_ROM_ADDR;
    // usleep(2);
    return v;
}

#endif // __MODCHIP_H__
