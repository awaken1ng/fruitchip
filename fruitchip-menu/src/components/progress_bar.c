#include <wchar.h>

#include <gsKit.h>

#include "components/font.h"
#include <constants.h>

void progress_bar_paint_center(GSGLOBAL *gs, float progress)
{
    float x1 = MARGIN_X;
    float y1 = gs->Height / 2.0;

    float x2 = gs->Width - x1;
    float y2 = y1 + 4.0;

    float w = (x2 - x1) * progress;
    x2 = x1 + w;

    gsKit_prim_sprite(
        gs,
        x1, y1,
        x2, y2,
        1,
        FG
    );
}

void progress_bar_paint_center_with_message(GSGLOBAL *gs, float progress, wchar_t *text)
{
    progress_bar_paint_center(gs, progress);

    float y = gs->Height / 2.0 + 4.0;
    font_print_centered(gs, y, 1, FG, text);
}
