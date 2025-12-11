#include <stdlib.h>

#include "libpad.h"

#include "modchip/version.h"

#include <scene/settings/update.h>
#include "scene/message.h"
#include "scene/settings.h"
#include "version.h"

static list_state_t list = {
    .start_item_idx = 0,
    .max_items = MAX_LIST_ITEMS_ON_SCREEN,
};

static u32 item_idx_menu_version;
static u32 item_idx_modchip_fw_version;
static u32 item_idx_modchip_bootloader_version;
static u32 item_idx_update_firmware;
static u32 item_idx_update_apps;

static void pop_scene(struct state *state)
{
    list_clear(&list);
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
            state->update_type = UPDATE_TYPE_FW;
            scene_switch_to_update_scanning(state);
            state->repaint = true;
        }
        else if (list.hilite_idx == item_idx_update_apps)
        {
            state->update_type = UPDATE_TYPE_APPS;
            scene_switch_to_update_scanning(state);
            state->repaint = true;
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
    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_settings;
    scene.paint_handler = scene_paint_handler_settings;
    superscene_push_scene(scene);

    if (!version_init_firmware())
        scene_switch_to_message(state, L"Failed to get firmware version");

    if (!version_init_bootloader())
        scene_switch_to_message(state, L"Failed to get bootloader version");

    array_list_item_init(list.items);

    list_item_t item;
    list_item_init(&item);

    item.left_text = wstring_new_static(L"Menu version");
    item.right_text = wstring_new_copied_cstr(version_get_menu());
    item_idx_menu_version = list_push_item(&list, item);

    item.left_text = wstring_new_static(L"Modchip FW version");
    item.right_text = wstring_new_copied_cstr(version_get_firmware());
    item_idx_modchip_fw_version = list_push_item(&list, item);

    item.left_text = wstring_new_static(L"Modchip bootloader version");
    item.right_text = wstring_new_copied_cstr(version_get_bootloader());
    item_idx_modchip_bootloader_version = list_push_item(&list, item);

    item.left_text = wstring_new_static(L"Update firmware");
    item.right_text = wstring_new_static(L">");
    item_idx_update_firmware = list_push_item(&list, item);

    item.left_text = wstring_new_static(L"Update apps");
    item.right_text = wstring_new_static(L">");
    item_idx_update_apps = list_push_item(&list, item);
}
