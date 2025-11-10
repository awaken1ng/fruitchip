#pragma once

#include <stdint.h>
#include <wchar.h>

#include <gsKit.h>

int font_init(GSGLOBAL *gsGlobal);

int font_print(GSGLOBAL *gsGlobal, float x, float y, int z, uint64_t color, const wchar_t *text);

int font_print_centered(GSGLOBAL *gsGlobal, float y, int z, uint64_t color, const wchar_t *text);

int font_print_aligned_right(GSGLOBAL *gsGlobal, float xoff, float y, int z, uint64_t color, const wchar_t *text);

float font_text_width(const wchar_t *text);

float font_text_height(const wchar_t *text);
