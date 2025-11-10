#pragma once

#include "gsKit.h"

#include <wchar.h>

#define GLYPH_CIRCLE   L"●"
#define GLYPH_CROSS    L"x"
#define GLYPH_TRIANGLE L"▲"
#define GLYPH_SQUARE   L"■"
#define GLYPH_SELECT   L"▬"
#define GLYPH_START    L"►"
#define GLYPH_UP       L"▴"
#define GLYPH_DOWN     L"▾"
#define GLYPH_LEFT     L"◂"
#define GLYPH_RIGHT    L"▸"

static const u64 TRIANGLE = GS_SETREG_RGBA(0x00, 0xFF, 0x40, 0x80); // #00ff40 -> #00ff7f
static const u64 SQUARE = GS_SETREG_RGBA(0xFF, 0x40, 0xFF, 0x80); // #ff40ff -> #ff7fff
static const u64 CROSS = GS_SETREG_RGBA(0x20, 0x60, 0xFF, 0x80); // #2060ff -> #3fbfff
static const u64 CIRCLE = GS_SETREG_RGBA(0xFF, 0x10, 0x10, 0x80); // #ff1010 -> #ff1f1f

struct button_guide_state {
    wchar_t *triangle;
    wchar_t *square;
    wchar_t *circle;
    wchar_t *cross;

    wchar_t *select;
    wchar_t *start;

    wchar_t *up;
    wchar_t *down;
    wchar_t *left;
    wchar_t *right;
};

void button_guide_draw(GSGLOBAL *gs, struct button_guide_state *state);
