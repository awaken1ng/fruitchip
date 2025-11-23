#pragma once

#include <wchar.h>

#include "tamtypes.h"
#include "gsKit.h"

#include "mlib/m-array.h"
#include "wstring.h"

typedef struct list_item
{
    wstring_t *left_text;
    wstring_t *right_text;
} list_item_t;

void list_item_init(list_item_t *obj);

void list_item_copy(list_item_t *obj, const list_item_t *src);

void list_item_free(list_item_t *obj);

ARRAY_DEF(
    array_list_item,
    list_item_t,
    (
        INIT(API_2(list_item_init)),
        INIT_SET(API_6(list_item_copy)),
        SET(API_6(list_item_copy)),
        CLEAR(API_2(list_item_free)),
    )
)

typedef struct list_state
{
    array_list_item_t items;
    u32 hilite_idx;
    u32 start_item_idx;
    u32 max_items;
} list_state_t;

#define MAX_LIST_ITEMS_ON_SCREEN 10

bool list_handle_input(list_state_t *state, int input);
void list_draw_items(GSGLOBAL *gs, list_state_t *state);

u32 list_push_item(list_state_t *state, const list_item_t item);
void list_clear(list_state_t *state);
void list_remove_item(list_state_t *state, u32 item_idx);
size_t list_len(list_state_t *state);
