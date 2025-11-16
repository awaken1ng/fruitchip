
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "libpad.h"

#include "modchip/apps.h"
#include "modchip/settings.h"

#include "../components/font.h"
#include "boot_options.h"
#include "superscene.h"
#include "../boot_list.h"
#include "../constants.h"
#include "../utils.h"

static wchar_t *ON = L"On";
static wchar_t *OFF = L"Off";

static struct scene_state {
    wstring_t *header;
    struct list_state list;
    union osdsys_settings settings;

    u32 item_idx_osdsys_skip_intro;
    u32 item_idx_osdsys_boot_browser;
    u32 item_idx_osdsys_skip_mc;
    u32 item_idx_osdsys_skip_hdd;
} scene_state;

static void pop_scene(struct state *state)
{
    bool settings_changed = scene_state.settings.value != state->osdsys.value;
    if (settings_changed)
        modchip_settings_set(MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS, state->osdsys.value);

    state->header = NULL;
    wstring_free(scene_state.header);

    while (scene_state.list.items_count) list_pop_item(&scene_state.list);

    superscene_pop_scene();
    state->repaint = true;

}

static void handle_toggle_option(struct state *state)
{
    u32 hilite_idx = scene_state.list.hilite_idx;

    bool value;
    if (hilite_idx == scene_state.item_idx_osdsys_skip_intro)
    {
        value = state->osdsys.field.skip_intro = !state->osdsys.field.skip_intro;
        if (state->osdsys.field.skip_intro)
        {
            state->osdsys.field.boot_browser = false;

            struct list_item *boot_browser = scene_state.list.items[scene_state.item_idx_osdsys_boot_browser];
            wstring_free(boot_browser->right_text);
            boot_browser->right_text = wstring_new_static(OFF);
        }
    }
    else if (hilite_idx == scene_state.item_idx_osdsys_boot_browser)
    {
        value = state->osdsys.field.boot_browser = !state->osdsys.field.boot_browser;
        if (state->osdsys.field.boot_browser)
        {
            state->osdsys.field.skip_intro = false;

            struct list_item *skip_intro = scene_state.list.items[scene_state.item_idx_osdsys_skip_intro];
            wstring_free(skip_intro->right_text);
            skip_intro->right_text = wstring_new_static(OFF);
        }
    }
    else if (hilite_idx == scene_state.item_idx_osdsys_skip_mc)
    {
        value = state->osdsys.field.skip_mc_update = !state->osdsys.field.skip_mc_update;
    }
    else if (hilite_idx == scene_state.item_idx_osdsys_skip_hdd)
    {
        value = state->osdsys.field.skip_hdd_update = !state->osdsys.field.skip_hdd_update;
    }
    else
    {
        return;
    }

    struct list_item *hilite = scene_state.list.items[hilite_idx];
    wstring_free(hilite->right_text);
    hilite->right_text = wstring_new_static(value ? ON : OFF);

    state->repaint = true;
}

static void scene_input_handler_options(struct state *state, int input)
{
    if (list_handle_input(&scene_state.list, input))
        state->repaint = true;

    else if (input & PAD_CIRCLE)
        pop_scene(state);

    else if (input & PAD_CROSS)
        handle_toggle_option(state);
}

static void scene_paint_handler_options(struct state *state)
{
    state->header = scene_state.header->data;

    list_draw_items(state->gs, &scene_state.list);

    superscene_clear_button_guide(state);
    state->button_guide.cross = L"Toggle";
    state->button_guide.circle = L"Return";
}

void scene_switch_to_options(struct state *state, u8 app_idx)
{
    wchar_t *name;

    if (app_idx == BOOT_ITEM_OSDSYS)
        name = L"OSDSYS";
    else
        name = wstring_data(state->boot_list.items[app_idx]->left_text);

    wchar_t header[39 + 255];
    snwprintf(header, 39 + 255, L"Options: %ls", name);

    wstring_free(scene_state.header);
    scene_state.header = wstring_new_copied_wstr(header);

    scene_state.list.hilite_idx = 0;
    scene_state.list.start_item_idx = 0;
    scene_state.list.max_items = MAX_LIST_ITEMS_ON_SCREEN;

    modchip_settings_get(MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS, &state->osdsys.value);
    scene_state.settings.value = state->osdsys.value;

    struct list_item item;
    u32 attr = state->apps_attr[app_idx];

    if (attr & MODCHIP_APPS_ATTR_OSDSYS)
    {
        item.left_text = wstring_new_static(L"Skip opening");
        item.right_text = wstring_new_static(state->osdsys.field.skip_intro ? ON : OFF);
        scene_state.item_idx_osdsys_skip_intro = list_push_item(&scene_state.list, &item);

        item.left_text = wstring_new_static(L"Boot into browser");
        item.right_text = wstring_new_static(state->osdsys.field.boot_browser ? ON : OFF);
        scene_state.item_idx_osdsys_boot_browser = list_push_item(&scene_state.list, &item);

        item.left_text = wstring_new_static(L"Skip memory card system update");
        item.right_text = wstring_new_static(state->osdsys.field.skip_mc_update ? ON : OFF);
        scene_state.item_idx_osdsys_skip_mc = list_push_item(&scene_state.list, &item);

        item.left_text = wstring_new_static(L"Skip hard drive system update");
        item.right_text = wstring_new_static(state->osdsys.field.skip_hdd_update ? ON : OFF);
        scene_state.item_idx_osdsys_skip_hdd = list_push_item(&scene_state.list, &item);
    }


    struct scene scene = {
        .input_handler = scene_input_handler_options,
        .paint_handler = scene_paint_handler_options,
    };
    superscene_push_scene(&scene);

    state->repaint = true;
}
