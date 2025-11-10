#include "button_guide.h"
#include "../components/font.h"
#include "../constants.h"

static float draw_button(
    GSGLOBAL *gs,
    const wchar_t *prefix,
    wchar_t *button_text,
    uint64_t color,
    float x,
    float y
)
{
    if (!button_text) return x;

    x -= font_text_width(button_text);
    font_print(gs, x, y, 1.0, FG, button_text);

    x -= font_text_width(prefix);
    font_print(gs, x, y, 1.0, color, prefix);

    x -= font_text_width(L"  ");

    return x;
}

void button_guide_draw(GSGLOBAL *gs, struct button_guide_state *state)
{
    float x = gs->Width - ITEM_TEXT_X_START;
    float y = BUTTON_GUIDE_START_Y + ITEM_TEXT_Y_OFFSET;

    x = draw_button(gs, GLYPH_CIRCLE   L" ", state->circle, CIRCLE, x, y);
    x = draw_button(gs, GLYPH_CROSS    L" ", state->cross, CROSS, x, y);
    x = draw_button(gs, GLYPH_TRIANGLE L" ", state->triangle, TRIANGLE, x, y);
    x = draw_button(gs, GLYPH_SQUARE   L" ", state->square, SQUARE, x, y);

    x = draw_button(gs, GLYPH_SELECT L" ", state->select, FG, x, y);
    x = draw_button(gs, GLYPH_START  L" ", state->start, FG, x, y);

    x = draw_button(gs, GLYPH_UP    L" ", state->up, FG, x, y);
    x = draw_button(gs, GLYPH_DOWN  L" ", state->down, FG, x, y);
    x = draw_button(gs, GLYPH_LEFT  L" ", state->left, FG, x, y);
    x = draw_button(gs, GLYPH_RIGHT L" ", state->right, FG, x, y);
}
