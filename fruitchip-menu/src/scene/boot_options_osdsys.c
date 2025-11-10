
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "libpad.h"

#include "modchip/settings.h"

#include "../components/font.h"
#include "boot_options_osdsys.h"
#include "superscene.h"
#include "../constants.h"

static wchar_t *ON = L"On";
static wchar_t *OFF = L"Off";

static struct list_state list = {
    .start_item_idx = 0,
    .max_items = MAX_LIST_ITEMS_ON_SCREEN,
};

#define ITEM_IDX_SKIP_INTRO 0
#define ITEM_IDX_BOOT_BROWSER 1
#define ITEM_IDX_SKIP_MC 2
#define ITEM_IDX_SKIP_HDD 3

static union osdsys_settings settings;

static void pop_scene(struct state *state)
{
    bool settings_changed = settings.value != state->osdsys.value;
    if (settings_changed)
        modchip_settings_set(MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS, state->osdsys.value);

    while (list.items_count) list_pop_item(&list);

    superscene_pop_scene();
    state->repaint = true;
}

static void handle_toggle_option(struct state *state)
{
    wstring_free(list.items[list.hilite_idx]->right_text);

    bool value;
    switch (list.hilite_idx)
    {
        case ITEM_IDX_SKIP_INTRO:
            value = state->osdsys.field.skip_intro = !state->osdsys.field.skip_intro;
            if (state->osdsys.field.skip_intro)
            {
                state->osdsys.field.boot_browser = false;
                wstring_free(list.items[ITEM_IDX_BOOT_BROWSER]->right_text);
                list.items[ITEM_IDX_BOOT_BROWSER]->right_text = wstring_new_static(OFF);
            }
            break;
        case ITEM_IDX_BOOT_BROWSER:
            value = state->osdsys.field.boot_browser = !state->osdsys.field.boot_browser;
            if (state->osdsys.field.boot_browser)
            {
                state->osdsys.field.skip_intro = false;
                wstring_free(list.items[ITEM_IDX_SKIP_INTRO]->right_text);
                list.items[ITEM_IDX_SKIP_INTRO]->right_text = wstring_new_static(OFF);
            }
            break;
        case ITEM_IDX_SKIP_MC:
            value = state->osdsys.field.skip_mc_update = !state->osdsys.field.skip_mc_update;
            break;
        case ITEM_IDX_SKIP_HDD:
            value = state->osdsys.field.skip_hdd_update = !state->osdsys.field.skip_hdd_update;
            break;
    }
    list.items[list.hilite_idx]->right_text = wstring_new_static(value ? ON : OFF);

    state->repaint = true;
}

static void scene_input_handler_options_osdsys(struct state *state, int input)
{

    if (list_handle_input(&list, input))
        state->repaint = true;

    else if (input & PAD_CIRCLE)
        pop_scene(state);

    else if (input & PAD_CROSS)
        handle_toggle_option(state);
}

static void scene_paint_handler_options_osdsys(struct state *state)
{
    state->header = L"OSDSYS options";

    list_draw_items(state->gs, &list);

    superscene_clear_button_guide(state);
    state->button_guide.cross = L"Toggle";
    state->button_guide.circle = L"Return";
}

void scene_switch_to_options_osdsys(struct state *state)
{
    modchip_settings_get(MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS, &state->osdsys.value);
    settings.value = state->osdsys.value;

    struct list_item item;

    item.left_text = wstring_new_static(L"Skip opening");
    item.right_text = wstring_new_static(state->osdsys.field.skip_intro ? ON : OFF);
    list_push_item(&list, &item);

    item.left_text = wstring_new_static(L"Boot into browser");
    item.right_text = wstring_new_static(state->osdsys.field.boot_browser ? ON : OFF);
    list_push_item(&list, &item);

    item.left_text = wstring_new_static(L"Skip memory card system update");
    item.right_text = wstring_new_static(state->osdsys.field.skip_mc_update ? ON : OFF);
    list_push_item(&list, &item);

    item.left_text = wstring_new_static(L"Skip hard drive system update");
    item.right_text = wstring_new_static(state->osdsys.field.skip_hdd_update ? ON : OFF);
    list_push_item(&list, &item);

    struct scene scene = {
        .input_handler = scene_input_handler_options_osdsys,
        .paint_handler = scene_paint_handler_options_osdsys,
    };
    superscene_push_scene(&scene);

    state->repaint = true;
}
