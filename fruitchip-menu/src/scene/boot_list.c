#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>

#include "elf-loader.h"
#include "kernel.h"
#include "libpad.h"
#include "sifcmd-common.h"
#include "timer.h"
#include "timer_alarm.h"

#include "modchip/fwfs.h"
#include "modchip/io.h"
#include "modchip/cmd.h"
#include "modchip/apps.h"
#include "modchip/settings.h"

#include "../components/font.h"
#include "../components/list.h"
#include "boot_options.h"
#include "message.h"
#include "settings.h"
#include "superscene.h"
#include "../boot_list.h"
#include "../constants.h"
#include "../state.h"
#include "../utils.h"

static void boot_osdsys(struct state *state)
{
    modchip_cmd(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK);
    SifExitCmd();

    int argc = 0;
    char *argv[4];
    if (state->osdsys.field.skip_intro) argv[argc++] = "BootClock";
    if (state->osdsys.field.boot_browser) argv[argc++] = "BootBrowser";
    if (state->osdsys.field.skip_mc_update) argv[argc++] = "SkipMc";
    if (state->osdsys.field.skip_hdd_update) argv[argc++] = "SkipHdd";
    ExecOSD(argc, argv);
}

static void boot_fwfs(struct state *state)
{
    u8 app_idx = state->boot_list.hilite_idx;

    int argc =  0;
    char *argv[4];

    u32 attr = state->apps_attr[app_idx];

    if (attr & MODCHIP_APPS_ATTR_DISABLE_NEXT_OSDSYS_HOOK)
    {
        modchip_cmd(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK);
    }

    if (attr & MODCHIP_APPS_ATTR_OSDSYS)
    {
        if (state->osdsys.field.skip_intro) argv[argc++] = "BootClock";
        if (state->osdsys.field.boot_browser) argv[argc++] = "BootBrowser";
        if (state->osdsys.field.skip_mc_update) argv[argc++] = "SkipMc";
        if (state->osdsys.field.skip_hdd_update) argv[argc++] = "SkipHdd";
    }

    char path[] = { 'f', 'w', 'f', 's', ':', FWFS_MODE_DATA_CHAR, app_idx };
    LoadELFFromFile(path, 0, NULL);
}

// region: input

static void boot_hilited_item(struct state *state)
{
    if (state->boot_list.hilite_idx == BOOT_ITEM_OSDSYS)
    {
        boot_osdsys(state);
    }
    else if (state->boot_list.hilite_idx >= BOOT_ITEM_FWFS)
    {
        boot_fwfs(state);
    }
}

static inline bool is_autoboot_item_hilited(struct state *state)
{
    return state->boot_list.hilite_idx == state->autoboot_item_idx;
}

static void set_autoboot_item_idx(struct state *state)
{
    if (is_autoboot_item_hilited(state)) return;

    bool failed = !modchip_settings_set(MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX, state->boot_list.hilite_idx);
    if (failed)
    {
        scene_switch_to_message(state, L"Failed to set autoboot item");
        return;
    }

    state->autoboot_item_idx = state->boot_list.hilite_idx;
    state->repaint = true;
}

void scene_input_handler_boot_list(struct state *state, int input)
{
    if (state->autoboot_countdown)
    {
        state->autoboot_countdown = false;
        state->repaint = true;
    }

    if (input & PAD_UP)
    {
        if (state->boot_list.hilite_idx == 0) state->boot_list.hilite_idx = state->boot_list.items_count - 1;
        else state->boot_list.hilite_idx -= 1;
        state->repaint = true;
    }
    else if (input & PAD_DOWN)
    {
        state->boot_list.hilite_idx += 1;
        if (state->boot_list.hilite_idx > state->boot_list.items_count - 1) state->boot_list.hilite_idx = 0;
        state->repaint = true;
    }

    else if (input & PAD_CROSS)
    {
        boot_hilited_item(state);
        state->repaint = true;
    }
    else if (input & PAD_TRIANGLE)
    {
        u8 app_idx = state->boot_list.hilite_idx;
        u32 attr = state->apps_attr[app_idx];
        if (attr)
            scene_switch_to_options(state, app_idx);
    }
    else if (input & PAD_SQUARE)
    {
        set_autoboot_item_idx(state);
    }

    else if (input & PAD_START)
    {
        scene_switch_to_settings(state);
    }
}

// endregion: input

// region: paint

static void handle_autoboot_countdown(struct state *state)
{
    if (!state->autoboot_countdown)
        return;

    if (state->autoboot_countdown_sec)
    {
        float y = ITEMS_Y_START + (ITEM_H * 10) + ITEM_TEXT_Y_OFFSET;

        wchar_t text[128];
        snwprintf(text, 128, L"Booting in %u seconds", state->autoboot_countdown_sec);
        font_print(state->gs, ITEM_TEXT_X_START, y, 1, FG, text);
    }
    else
    {
        boot_hilited_item(state);
    }
}

static void draw_button_guide(struct state *state)
{
    superscene_clear_button_guide(state);

    state->button_guide.cross = L"Launch";

    u8 app_idx = state->boot_list.hilite_idx;
    u32 attr = state->apps_attr[app_idx];

    state->button_guide.start = L"Settings";

    if (attr)
        state->button_guide.triangle = L"Options";

    if (!is_autoboot_item_hilited(state))
        state->button_guide.square = L"Set autoboot";
}

void scene_paint_handler_boot_list(struct state *state)
{
    state->header = L"Boot menu";
    list_draw_items(state->gs, &state->boot_list);
    handle_autoboot_countdown(state);
    draw_button_guide(state);
}

// endregion: paint

static u64 update_autoboot_countdown(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
    (void)alarm_id;
    (void)scheduled_time;
    (void)actual_time;
    (void)pc_value;

    struct state *state = arg;

    if (state->autoboot_countdown_sec)
        state->autoboot_countdown_sec -= 1;

    iReleaseTimerAlarm(state->autoboot_countdown_alarm);
    state->autoboot_countdown_alarm = -1;
    if (state->autoboot_countdown && state->autoboot_countdown_sec)
        state->autoboot_countdown_alarm = iSetTimerAlarm(Sec2TimerBusClock(1), &update_autoboot_countdown, arg);

    state->repaint = true;

    ExitHandler();
    return 0;
}

void scene_switch_to_boot_list(struct state *state)
{
    struct scene scene = {
        .input_handler = scene_input_handler_boot_list,
        .paint_handler = scene_paint_handler_boot_list,
    };
    superscene_push_scene(&scene);

    bool ret = apps_list_populate(state);
    if (!ret)
    {
        scene_switch_to_message(state, L"Failed to load apps list");
    }

#ifndef NDEBUG
    printf("autoboot %i\n", state->autoboot);
    printf("autoboot_item_idx %i\n", state->autoboot_item_idx);
    printf("autoboot_delay_sec %i\n", state->autoboot_delay_sec);
#endif

    if (ret && state->autoboot)
    {
        state->autoboot_countdown = true;
        state->autoboot_countdown_sec = state->autoboot_delay_sec;
        state->autoboot_countdown_alarm = SetTimerAlarm(Sec2TimerBusClock(1), &update_autoboot_countdown, state);
    }

    state->repaint = true;
}
