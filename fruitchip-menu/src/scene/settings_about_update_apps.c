
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "libpad.h"

#include "modchip/flash.h"
#include "modchip/version.h"

#include "uf2.h"
#include "components/font.h"
#include "scene/message.h"
#include "scene/settings.h"
#include "constants.h"
#include "git_version.h"
#include "update.h"
#include "utils.h"

static struct update_state {
    list_state_t list;
} update_state;

#define ITEM_IDX_NO 0
#define ITEM_IDX_YES 1

static void pop_scene(struct state *state)
{
    list_clear(&update_state.list);
    superscene_pop_scene();
    state->repaint = true;
}

// region: input

static void scene_input_handler_file_found(struct state *state, int input)
{
    if (!state->update_complete && list_handle_input(&update_state.list, input))
    {
        state->repaint = true;
    }
    else if (input & PAD_CIRCLE)
    {
        pop_scene(state);
    }
    else if (!state->update_complete && input & PAD_CROSS)
    {
        switch (update_state.list.hilite_idx)
        {
            case ITEM_IDX_NO:
                pop_scene(state);
                break;
            case ITEM_IDX_YES:
                update_state.list.hilite_idx = 0;
                update_start_apps_update(state);
                break;
        }
    }
}

static void scene_input_handler_file_not_found(struct state *state, int input)
{
    if (input & PAD_CIRCLE)
    {
        pop_scene(state);
    }
    else if (input & PAD_CROSS)
    {
        state->update_file_present = update_is_apps_update_file_present();
        state->repaint = true;
    }
}

static void scene_input_handler_settings_update(struct state *state, int input)
{
    if (state->update_file_present)
        scene_input_handler_file_found(state, input);
    else
        scene_input_handler_file_not_found(state, input);
}

// endregion: input

// region: paint

static void scene_paint_handler_updating(struct state *state)
{
    float x1 = MARGIN_X;
    float y1 = state->gs->Height / 2.0;

    float x2 = state->gs->Width - x1;
    float y2 = y1 + 4.0;

    float progress = (float)state->update_block_current / (float)state->update_blocks_total;
    float w = (x2 - x1) * progress;
    x2 = x1 + w;

    gsKit_prim_sprite(
        state->gs,
        x1, y1,
        x2, y2,
        1,
        FG
    );

    wchar_t text[64];
    snwprintf(text, 32, L"Blocks written: %u/%u", state->update_block_current, state->update_blocks_total);
    font_print_centered(state->gs, y2 + 2, 1, FG, text);
}

static void scene_paint_handler_file_found(struct state *state)
{
    if (state->update_complete)
    {
        wchar_t *line4 = L"Update complete";
        font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG, line4);
    }
    else
    {
        wchar_t *line1 = L"Do not reset the console while update is in progress";
        font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG, line1);
    }

    if (!state->update_complete)
    {
        wchar_t *line4 = L"Start the update?";
        font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(2), 1, FG, line4);
        list_draw_items(state->gs, &update_state.list);
        state->button_guide.cross = L"Select";
    }

    state->button_guide.circle = L"Return";
}

static void scene_paint_handler_file_not_found(struct state *state)
{
    wchar_t *line1_part1 = L"Copy update file to USB drive to ";
    wchar_t *line1_part2 = L"" APPS_UPDATE_PATH;
    float line1_part1_w = font_text_width(line1_part1);

    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG, line1_part1);
    // double draw to bold the path
    font_print(state->gs, ITEM_TEXT_X_START + line1_part1_w, ITEM_TEXT_Y(0), 1, FG, line1_part2);
    font_print(state->gs, ITEM_TEXT_X_START + line1_part1_w, ITEM_TEXT_Y(0), 1, FG, line1_part2);

    wchar_t *line2 = L"Connect USB drive with update file and press ";
    float line2_w = font_text_width(line2);

    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(1), 1, FG, line2);
    font_print(state->gs, ITEM_TEXT_X_START + line2_w, ITEM_TEXT_Y(1), 1, CROSS, GLYPH_CROSS);

    state->button_guide.cross = L"Rescan";
    state->button_guide.circle = L"Return";
}

static void scene_paint_handler_settings_update(struct state *state)
{
    superscene_clear_button_guide(state);

    if (state->update_in_progress)
    {
        scene_paint_handler_updating(state);
    }
    else if (state->update_file_present)
    {
        scene_paint_handler_file_found(state);
    }
    else
    {
        scene_paint_handler_file_not_found(state);
    }
}

// endregion: paint

void scene_switch_to_settings_about_update_apps(struct state *state)
{
    state->header = L"Update apps";

    state->update_file_present = update_is_apps_update_file_present();
    state->update_complete = false;
    state->update_in_progress = false;

    array_list_item_init(update_state.list.items);
    update_state.list.hilite_idx = 0;
    update_state.list.start_item_idx = 3;
    update_state.list.max_items = MAX_LIST_ITEMS_ON_SCREEN;

    list_item_t item;
    list_item_init(&item);

    item.left_text = wstring_new_static(L"No");
    item.right_text = NULL;
    list_push_item(&update_state.list, item);

    item.left_text = wstring_new_static(L"Yes");
    item.right_text = NULL;
    list_push_item(&update_state.list, item);

    struct scene scene = {
        .input_handler = scene_input_handler_settings_update,
        .paint_handler = scene_paint_handler_settings_update,
    };
    superscene_push_scene(&scene);

    state->repaint = true;
}
