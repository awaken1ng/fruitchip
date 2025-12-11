#include <sleep.h>

#include <components/font.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <constants.h>
#include <unistd.h>
#include <update.h>
#include <utils.h>

static enum update_type update_type;

static void scene_tick_handler_update_scanning(struct state *state)
{
    scene_paint_handler_superscene(state);
    superscene_pop_scene();

    u64 start_us = clock_us();
    update_file_is_present(update_type)
        ? scene_switch_to_update_found(update_type)
        : scene_switch_to_update_not_found(update_type);
    u64 end_us = clock_us();

    u64 took_us = end_us - start_us;
    static const u64 min_screen_time_us = 250000; // 250 ms
    if (took_us < min_screen_time_us)
    {
        sleep_us(min_screen_time_us - took_us);
    }

    state->repaint = true;
}

static void scene_paint_handler_update_scanning(struct state *state)
{
    font_print(
        state->gs,
        ITEM_TEXT_X_START,
        ITEM_TEXT_Y(0),
        1,
        FG,
        L"Checking update file"
    );

    superscene_clear_button_guide(state);
}

void scene_switch_to_update_scanning(struct state *state, enum update_type ty)
{
    scene_t scene;
    scene_init(&scene);
    scene.tick_handler = scene_tick_handler_update_scanning;
    scene.paint_handler = scene_paint_handler_update_scanning;
    superscene_push_scene(scene);

    update_type = ty;

    switch (update_type)
    {
        case UPDATE_TYPE_FW:
        {
            state->header = L"Update firmware";
            break;
        }
        case UPDATE_TYPE_APPS:
        {
            state->header = L"Update apps";
            break;
        }
    }
}
