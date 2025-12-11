#include <libpad.h>

#include <components/font.h>
#include <scene/superscene.h>
#include <constants.h>

static void scene_input_handler_update_complete(struct state *state, int input)
{
    if (input & PAD_CIRCLE)
    {
        superscene_pop_scene();
        state->repaint = true;
    }
}

static void scene_paint_handler_update_complete(struct state *state)
{
    font_print(state->gs, ITEM_TEXT_X_START, ITEM_TEXT_Y(0), 1, FG,  L"Update complete");

    superscene_clear_button_guide(state);
    state->button_guide.circle = L"Return";
}

void scene_switch_to_update_complete()
{
    scene_t scene;
    scene_init(&scene);
    scene.input_handler = scene_input_handler_update_complete;
    scene.paint_handler = scene_paint_handler_update_complete;
    superscene_push_scene(scene);
}
