#pragma once

#include <malloc.h>

#include <gsKit.h>

#ifdef NDEBUG
    #define print_debug(x...) printf(x)
#else
    #define print_debug(x...)
#endif

int gsKit_texture_png_from_memory(GSGLOBAL *gsGlobal, GSTEXTURE *Texture, void *buf, size_t size);

int snwprintf(wchar_t *buffer, size_t n, const wchar_t * format, ...);

u64 clock_us(void);

u32 div_ceil_u32(u32 x, u32 y);
