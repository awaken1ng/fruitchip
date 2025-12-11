#include <libpad.h>

#include <components/font.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <constants.h>

static enum update_type update_type;

static void scene_input_handler_update_not_found(struct state *state, int input)
{
    if (input & PAD_CIRCLE)
    {
        superscene_pop_scene();
        state->repaint = true;
    }
    else if (input & PAD_CROSS)
    {
        superscene_pop_scene();
        scene_switch_to_update_scanning(state, update_type);
        state->repaint = true;
    }
}

static void scene_paint_handler_update_not_found(struct state *state)
{

    wchar_t *line1_part1 = L"Copy update file to USB drive to ";
    wchar_t *line1_part2;
    float line1_part1_w = font_text_width(line1_part1);

    switch (update_type)
    {
        case UPDATE_TYPE_FW:
            line1_part2 = L"" FW_UPDATE_PATH; break;
        case UPDATE_TYPE_APPS:
            line1_part2 = L"" APPS_UPDATE_PATH; break;
    }

    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG, line1_part1);
    // double draw to bold the path
    font_print(state->gs, ITEM_TEXT_X_START + line1_part1_w, ITEM_TEXT_Y(0), 1, FG, line1_part2);
    font_print(state->gs, ITEM_TEXT_X_START + line1_part1_w, ITEM_TEXT_Y(0), 1, FG, line1_part2);

    wchar_t *line2 = L"Connect USB drive with update file and press ";
    float line2_w = font_text_width(line2);

    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(1), 1, FG, line2);
    font_print(state->gs, ITEM_TEXT_X_START + line2_w, ITEM_TEXT_Y(1), 1, CROSS, GLYPH_CROSS);

    superscene_clear_button_guide(state);
    state->button_guide.cross = L"Rescan";
    state->button_guide.circle = L"Return";
}

void scene_switch_to_update_not_found(enum update_type ty)
{
    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_update_not_found;
    scene.paint_handler = scene_paint_handler_update_not_found;
    superscene_push_scene(scene);

    update_type = ty;
}