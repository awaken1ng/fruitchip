#pragma once

#include <wchar.h>

#include <gsKit.h>

void progress_bar_paint_center(GSGLOBAL *gs, float progress);
void progress_bar_paint_center_with_message(GSGLOBAL *gs, float progress, wchar_t *text);
