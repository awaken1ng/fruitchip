#include <unistd.h>

#include <modchip/flash.h>

#include <components/font.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <scene/message.h>
#include <constants.h>
#include <utils.h>

static void scene_tick_handler_update_rebooting(struct state *state)
{
    scene_paint_handler_superscene(state);
    superscene_pop_scene();

    state->repaint = true;

    enum modchip_reboot_mode reboot_mode;
    switch (state->update_type)
    {
        case UPDATE_TYPE_FW:
            reboot_mode = MODCHIP_REBOOT_MODE_UPDATE;
            break;
        case UPDATE_TYPE_APPS:
            reboot_mode = MODCHIP_REBOOT_MODE_SIMPLE;
            break;
    }

    u64 reboot_start = clock_us();
    u64 reboot_deadline = reboot_start + (1000000 * 30); // 30s
    if (!modchip_reboot_with_retry(reboot_mode, MODCHIP_CMD_RETRIES))
    {
        scene_switch_to_message(state, L"Failed to reboot the modchip");
        return;
    }

    // wait for reboot
    while (true)
    {
        if (modchip_ping()) break;

        sleep(1);
        if (clock_us() > reboot_deadline)
        {
            scene_switch_to_message(state, L"Failed to reboot the modchip, timed out");
            return;
        }
    }

    scene_switch_to_update_complete();
}

static void scene_paint_handler_update_rebooting(struct state *state)
{
    float y1 = state->gs->Height / 2.0;
    float y2 = y1 + 4.0;

    wchar_t *text2 = L"Rebooting modchip";
    font_print_centered(state->gs, y2 + 2, 1, FG, text2);

    superscene_clear_button_guide(state);
}

void scene_switch_to_update_rebooting()
{
    scene_t scene;
    scene_init(&scene);
    scene.tick_handler = scene_tick_handler_update_rebooting;
    scene.paint_handler = scene_paint_handler_update_rebooting;
    superscene_push_scene(scene);
}