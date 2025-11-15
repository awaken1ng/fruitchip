#include <stdio.h>
#include <stdlib.h>

#include "libpad.h"

#include "minmax.h"

#include "list.h"
#include "../constants.h"
#include "../components/font.h"

bool list_handle_input(struct list_state *state, int input)
{
    if (input & PAD_UP)
    {
        if (state->hilite_idx == 0) state->hilite_idx = state->items_count - 1;
        else state->hilite_idx -= 1;
        return true;
    }
    else if (input & PAD_DOWN)
    {
        state->hilite_idx += 1;
        if (state->hilite_idx > state->items_count - 1) state->hilite_idx = 0;
        return true;
    }

    return false;
}

// void list_draw_header(GSGLOBAL *gs, struct list_state *state)
// {
//     if (!state->header)
//         return;
//
//     float y1 = HEADER_Y_START;
//     float y2 = y1 + ITEM_H;
//
//     // clear
//     gsKit_prim_sprite(
//         gs,
//         MARGIN_X, y1,
//         gs->Width - MARGIN_X, y2,
//         1,
//         BG
//     );
//
//     font_print(gs, ITEM_TEXT_X_START, y1 + ITEM_TEXT_Y_OFFSET, 1, FG, state->header);
//
//     // draw separator line on top of the first list item
//     gsKit_prim_sprite(
//         gs,
//         MARGIN_X, y2,
//         gs->Width - MARGIN_X, y2 + HEADER_H,
//         1,
//         FG
//     );
// }

static void list_draw_item(GSGLOBAL *gs, float y, struct list_item *item, bool hilite)
{
    float y2 = y + ITEM_H;

    // clear
    gsKit_prim_sprite(
        gs,
        MARGIN_X, y,
        gs->Width - MARGIN_X, y2,
        1,
        hilite ? FG : BG
    );

    font_print(
        gs,
        ITEM_TEXT_X_START, y + ITEM_TEXT_Y_OFFSET,
        1,
        hilite ? BG : FG,
        wstring_data(item->left_text)
    );

    if (item->right_text)
    {
        font_print_aligned_right(gs,
            ITEM_TEXT_X_START, y + ITEM_TEXT_Y_OFFSET,
            1,
            hilite ? BG : FG,
            wstring_data(item->right_text)
        );
    }

}

void list_draw_items(GSGLOBAL *gs, struct list_state *state)
{
    u32 start_idx, end_idx;

    if (!state->items_count)
        return;

    if (state->hilite_idx < state->max_items)
    {
        start_idx = 0;
        end_idx = MIN(state->max_items, state->items_count) - 1;
    }
    else
    {
        start_idx = state->hilite_idx - (state->max_items - 1);
        end_idx = state->hilite_idx;
    }

    float start_y = ITEM_Y(state->start_item_idx);
    float scrollbar_end_y = start_y + (ITEM_H * state->max_items);
    float scrollbar_max_h = scrollbar_end_y - start_y;
    float scrollbar_x = gs->Width;
    float scrollbar_h = scrollbar_max_h / state->items_count * state->max_items;
    float scrollbar_y1 = start_y + (scrollbar_max_h / state->items_count * start_idx);
    float scrollbar_y2 = scrollbar_y1 + scrollbar_h;

    u32 item_screen_idx = 0;

    while (true)
    {
        if (start_idx > end_idx)
            break;

        if (state->items[start_idx]->left_text)
        {
            list_draw_item(
                gs,
                start_y + (ITEM_H * item_screen_idx),
                state->items[start_idx],
                start_idx == state->hilite_idx
            );
        }

        item_screen_idx += 1;
        start_idx += 1;
    }

    if (state->items_count > state->max_items)
    {
        // scrollbar
        gsKit_prim_sprite(
            gs,
            scrollbar_x, scrollbar_y1,
            scrollbar_x - ITEMS_SCROLLBAR_W, scrollbar_y2,
            1,
            FG
        );
    }
}

u32 list_push_item(struct list_state *state, struct list_item *item)
{
    if (state->items_count == 0)
        state->items = malloc(sizeof(*item));

    u32 idx = state->items_count;

    state->items_count += 1;
    state->items = realloc(state->items, sizeof(*state->items) * state->items_count);

    state->items[idx] = malloc(sizeof(*item));
    memcpy(state->items[idx], item, sizeof(*item));

    return idx;
}

void list_pop_item(struct list_state *state)
{
    if (!state->items_count) return;

    state->items_count -= 1;

    struct list_item *last_item = state->items[state->items_count];
    wstring_free(last_item->left_text);
    wstring_free(last_item->right_text);
    free(last_item);

    size_t new_size = sizeof(*state->items) * state->items_count;
    if (new_size == 0)
        free(state->items);
    else
        state->items = realloc(state->items, new_size);
}
