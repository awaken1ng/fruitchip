#include <stdio.h>
#include <stdlib.h>

#include "libpad.h"

#include "minmax.h"

#include "components/list.h"
#include "components/font.h"
#include "constants.h"
#include "utils.h"

// region: list item

void list_item_init(list_item_t *obj)
{
    obj->left_text = NULL;
    obj->right_text = NULL;
}

void list_item_copy(list_item_t *obj, const list_item_t *src)
{
    list_item_init(obj);

    if (src->left_text)
        obj->left_text = wstring_new_copied_wstr(wstring_data(src->left_text));

    if (src->right_text)
        obj->right_text = wstring_new_copied_wstr(wstring_data(src->right_text));
}

void list_item_free(list_item_t *obj)
{
    wstring_free(obj->left_text);
    wstring_free(obj->right_text);
}

// endregion: list item

bool list_handle_input(list_state_t *state, int input)
{
    if (input & PAD_UP)
    {
        if (state->hilite_idx == 0) state->hilite_idx = list_len(state) - 1;
        else state->hilite_idx -= 1;
        return true;
    }
    else if (input & PAD_DOWN)
    {
        state->hilite_idx += 1;
        if (state->hilite_idx > list_len(state) - 1) state->hilite_idx = 0;
        return true;
    }

    return false;
}

static void list_draw_item(GSGLOBAL *gs, float y, list_item_t *item, bool hilite)
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

void list_draw_items(GSGLOBAL *gs, list_state_t *state)
{
    u32 start_idx, end_idx;

    if (!list_len(state))
        return;

    if (state->hilite_idx < state->max_items)
    {
        start_idx = 0;
        end_idx = MIN(state->max_items, list_len(state)) - 1;
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
    float scrollbar_h = scrollbar_max_h / list_len(state) * state->max_items;
    float scrollbar_y1 = start_y + (scrollbar_max_h / list_len(state) * start_idx);
    float scrollbar_y2 = scrollbar_y1 + scrollbar_h;

    u32 item_screen_idx = 0;

    while (true)
    {
        if (start_idx > end_idx)
            break;

        if (array_list_item_get(state->items, start_idx)->left_text)
        {
            list_draw_item(
                gs,
                start_y + (ITEM_H * item_screen_idx),
                array_list_item_get(state->items, start_idx),
                start_idx == state->hilite_idx
            );
        }

        item_screen_idx += 1;
        start_idx += 1;
    }

    if (list_len(state) > state->max_items)
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

u32 list_push_item(list_state_t *state, const list_item_t item)
{
    if (!list_len(state))
        array_list_item_init(state->items);

    u32 idx = list_len(state);
    array_list_item_push_back(state->items, item);

    return idx;
}

void list_remove_item(list_state_t *state, u32 idx)
{
    array_list_item_remove_v(state->items, idx, idx + 1);

    while (state->hilite_idx >= list_len(state))
    {
        state->hilite_idx -= 1;
    }
}

void list_clear(list_state_t *state)
{
    array_list_item_clear(state->items);
    state->hilite_idx = 0;
}

size_t list_len(list_state_t *state)
{
    return state->items->size;
}
