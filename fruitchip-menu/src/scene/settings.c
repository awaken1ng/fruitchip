#include <stdlib.h>

#include "libpad.h"

#include "modchip/settings.h"
#include "minmax.h"

#include "scene/message.h"
#include "scene/settings.h"
#include "utils.h"

static list_state_t list = {
    .start_item_idx = 0,
    .max_items = MAX_LIST_ITEMS_ON_SCREEN,
};

static bool adjusting_autoboot_delay = false;

static struct {
    bool autoboot: 1;
    bool autoboot_delay: 1;
} needs_saving = { 0 };

#define ITEM_IDX_AUTOBOOT 0
#define ITEM_IDX_AUTOBOOT_DELAY 1
#define ITEM_IDX_ABOUT 2

#define AUTOBOOT_DELAY_MIN 1
#define AUTOBOOT_DELAY_MAX 90
#define AUTOBOOT_DELAY_STEP_SMALL 1
#define AUTOBOOT_DELAY_STEP_BIG 10

static void save_settings(struct state *state)
{
    bool failed = false;
    if (needs_saving.autoboot)
    {
        failed |= modchip_settings_set(MODCHIP_SETTINGS_MENU_AUTOBOOT, state->autoboot) != true;
        needs_saving.autoboot = false;
    }

    if (needs_saving.autoboot_delay)
    {
        failed |= modchip_settings_set(MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY, state->autoboot_delay_sec) != true;
        needs_saving.autoboot_delay = false;
    }

    if (failed)
        scene_switch_to_message(state, L"Failed to save settings");
}

static void update_autoboot_text(struct state *state)
{
    list_item_t *autoboot = array_list_item_get(list.items, ITEM_IDX_AUTOBOOT);
    wstring_free(autoboot->right_text);
    autoboot->right_text = wstring_new_static(state->autoboot ? L"On" : L"Off");
}

static void update_autoboot_delay_text(struct state *state)
{
    list_item_t *autoboot_delay = array_list_item_get(list.items, ITEM_IDX_AUTOBOOT_DELAY);

    wstring_free(autoboot_delay->right_text);

    autoboot_delay->right_text = wstring_new_allocated(64);

    if (state->autoboot_delay_sec == 1)
        snwprintf(wstring_data(autoboot_delay->right_text), 64, L"%u second", state->autoboot_delay_sec);
    else
        snwprintf(wstring_data(autoboot_delay->right_text), 64, L"%u seconds", state->autoboot_delay_sec);
}

static void pop_scene(struct state *state)
{
    superscene_pop_scene();
    list_clear(&list);
    state->repaint = true;
}

static void draw_button_guide(struct state *state)
{
    superscene_clear_button_guide(state);

    state->button_guide.circle = L"Return";

    switch (list.hilite_idx)
    {
        case ITEM_IDX_AUTOBOOT:
            state->button_guide.cross = L"Toggle";
            break;
        case ITEM_IDX_AUTOBOOT_DELAY:
            if (adjusting_autoboot_delay)
            {
                state->button_guide.right = L"+10";
                state->button_guide.left = L"-10";
                state->button_guide.down = L"-1";
                state->button_guide.up = L"+1";
            }
            else
            {
                state->button_guide.cross = L"Adjust";
            }
            break;
        case ITEM_IDX_ABOUT:
            state->button_guide.cross = L"Open";
            break;
    }
}

static void scene_input_handler_settings(struct state *state, int input)
{
    if (adjusting_autoboot_delay)
    {
        if (input & PAD_UP)
        {
            state->autoboot_delay_sec += AUTOBOOT_DELAY_STEP_SMALL;
            if (state->autoboot_delay_sec > AUTOBOOT_DELAY_MAX) state->autoboot_delay_sec = AUTOBOOT_DELAY_MIN;
            update_autoboot_delay_text(state);
            state->repaint = true;
            needs_saving.autoboot_delay = true;
        }
        else if (input & PAD_DOWN)
        {
            state->autoboot_delay_sec -= AUTOBOOT_DELAY_STEP_SMALL;
            if (state->autoboot_delay_sec < AUTOBOOT_DELAY_MIN) state->autoboot_delay_sec = AUTOBOOT_DELAY_MAX;
            state->autoboot_delay_sec = CLAMP(state->autoboot_delay_sec, AUTOBOOT_DELAY_MIN, AUTOBOOT_DELAY_MAX);
            update_autoboot_delay_text(state);
            state->repaint = true;
            needs_saving.autoboot_delay = true;
        }
        else if (input & PAD_RIGHT)
        {
            state->autoboot_delay_sec += AUTOBOOT_DELAY_STEP_BIG;
            if (state->autoboot_delay_sec > AUTOBOOT_DELAY_MAX) state->autoboot_delay_sec = AUTOBOOT_DELAY_MIN;
            update_autoboot_delay_text(state);
            state->repaint = true;
            needs_saving.autoboot_delay = true;
        }
        else if (input & PAD_LEFT)
        {
            state->autoboot_delay_sec -= AUTOBOOT_DELAY_STEP_BIG;
            if (state->autoboot_delay_sec < AUTOBOOT_DELAY_MIN) state->autoboot_delay_sec = AUTOBOOT_DELAY_MAX;
            state->autoboot_delay_sec = CLAMP(state->autoboot_delay_sec, AUTOBOOT_DELAY_MIN, AUTOBOOT_DELAY_MAX);
            update_autoboot_delay_text(state);
            state->repaint = true;
            needs_saving.autoboot_delay = true;
        }
        else if (input & PAD_CIRCLE)
        {
            adjusting_autoboot_delay = false;
            save_settings(state);
            state->repaint = true;
        }
    }
    else
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
            switch (list.hilite_idx)
            {
                case ITEM_IDX_AUTOBOOT:
                    state->autoboot = !state->autoboot;
                    needs_saving.autoboot = true;
                    save_settings(state);
                    update_autoboot_text(state);
                    state->repaint = true;
                    break;
                case ITEM_IDX_AUTOBOOT_DELAY:
                    adjusting_autoboot_delay = !adjusting_autoboot_delay;
                    state->repaint = true;
                    break;
                case ITEM_IDX_ABOUT:
                    scene_switch_to_settings_about(state);
                    state->repaint = true;
                    break;
            }
        }
    }
}

static void scene_paint_handler_settings(struct state *state)
{
    state->header = L"Settings";
    list_draw_items(state->gs, &list);
    draw_button_guide(state);
}

void scene_switch_to_settings(struct state *state)
{
    array_list_item_init(list.items);

    list_item_t item;
    list_item_init(&item);

    item.left_text = wstring_new_static(L"Autoboot");
    item.right_text = NULL;
    list_push_item(&list, item);
    update_autoboot_text(state);

    item.left_text = wstring_new_static(L"Autoboot delay");
    item.right_text = NULL;
    list_push_item(&list, item);
    update_autoboot_delay_text(state);

    item.left_text = wstring_new_static(L"About");
    item.right_text = wstring_new_static(L">");
    list_push_item(&list, item);

    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_settings;
    scene.paint_handler = scene_paint_handler_settings;
    superscene_push_scene(scene);
}
