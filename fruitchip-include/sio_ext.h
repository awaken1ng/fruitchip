#pragma once

#include <stdint.h>

#include <sio.h>

void sio_putxn_u8(u8 x)
{
    uint8_t lb = x >> 4;
    uint8_t rb = x & 0xf;

    char lc = lb < 10 ? '0' + lb : 'a' + lb - 10;
    char rc = rb < 10 ? '0' + rb : 'a' + rb - 10;

    sio_putc(lc);
    sio_putc(rc);
}

void sio_putxn_u32(u32 x)
{
    sio_putxn_u8(x >> 24);
    sio_putxn_u8(x >> 16);
    sio_putxn_u8(x >> 8);
    sio_putxn_u8(x >> 0);
}

#define sio_putxn(x) _Generic((x), u8: sio_putxn_u8, \
                                   u32: sio_putxn_u32)(x)
