#pragma once

#include <wchar.h>

#include "tamtypes.h"
#include "gsKit.h"

#include "wstring.h"

struct list_item
{
    wstring_t *left_text;
    wstring_t *right_text;
};

struct list_state
{
    // wchar_t *header;
    struct list_item **items;
    u32 items_count;
    u32 hilite_idx;

    u32 start_item_idx;
    u32 max_items;
};

#define MAX_LIST_ITEMS_ON_SCREEN 10

bool list_handle_input(struct list_state *state, int input);
// void list_draw_header(GSGLOBAL *gs, struct list_state *state);
void list_draw_items(GSGLOBAL *gs, struct list_state *state);
u32 list_push_item(struct list_state *state, struct list_item *item);
void list_pop_item(struct list_state *state);
