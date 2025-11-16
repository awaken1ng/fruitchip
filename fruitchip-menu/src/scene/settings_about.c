#include <stdlib.h>

#include "libpad.h"

#include "modchip/version.h"
#include "settings.h"
#include "version.h"

static struct list_state list = {
    .start_item_idx = 0,
    .max_items = MAX_LIST_ITEMS_ON_SCREEN,
};

static u32 item_idx_menu_version;
static u32 item_idx_modchip_fw_version;
static u32 item_idx_update_firmware;
static u32 item_idx_update_apps;

static void pop_scene(struct state *state)
{
    while (list.items_count) list_pop_item(&list);
    superscene_pop_scene();
    state->repaint = true;
}

static void scene_input_handler_settings(struct state *state, int input)
{
    if (list_handle_input(&list, input))
    {
        state->repaint = true;
    }
    else if (input & PAD_CIRCLE)
    {
        pop_scene(state);
    }
    else if (input & PAD_CROSS)
    {
        if (list.hilite_idx == item_idx_update_firmware)
        {
            scene_switch_to_settings_about_update_fw(state);
        }
        else if (list.hilite_idx == item_idx_update_apps)
        {
            scene_switch_to_settings_about_update_apps(state);
        }
    }
}

static void scene_paint_handler_settings(struct state *state)
{
    state->header = L"About";

    list_draw_items(state->gs, &list);

    superscene_clear_button_guide(state);
    state->button_guide.circle = L"Return";

    if (list.hilite_idx == item_idx_update_firmware ||
        list.hilite_idx == item_idx_update_apps)
    {
        state->button_guide.cross = L"Open";
    }
}

void scene_switch_to_settings_about(struct state *state)
{
    list.hilite_idx = 0;

    struct list_item item;

    item.left_text = wstring_new_static(L"Menu version");
    item.right_text = wstring_new_copied_cstr(GIT_REV);
    item_idx_menu_version = list_push_item(&list, &item);

    char fw_git_rev[9] = "N/A";
    modchip_git_rev(fw_git_rev);

    item.left_text = wstring_new_static(L"Modchip FW version");
    item.right_text = wstring_new_copied_cstr(fw_git_rev);
    item_idx_modchip_fw_version = list_push_item(&list, &item);

    item.left_text = wstring_new_static(L"Update firmware");
    item.right_text = wstring_new_static(L">");
    item_idx_update_firmware = list_push_item(&list, &item);

    item.left_text = wstring_new_static(L"Update apps");
    item.right_text = wstring_new_static(L">");
    item_idx_update_apps = list_push_item(&list, &item);

    struct scene scene = {
        .input_handler = scene_input_handler_settings,
        .paint_handler = scene_paint_handler_settings,
    };
    superscene_push_scene(&scene);

    state->repaint = true;
}
