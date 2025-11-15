#pragma once

#include <wchar.h>

#include <tamtypes.h>
#include <gsKit.h>
#include "input/pad.h"
#include "components/list.h"
#include "components/button_guide.h"

union osdsys_settings {
    struct {
        bool skip_intro : 1;
        bool boot_browser : 1;
        bool skip_mc_update : 1;
        bool skip_hdd_update : 1;
        u32 unused : 29;
    } field;
    u32 value;
};

struct state {
    GSGLOBAL *gs;
    bool repaint;

    // superscene
    wchar_t *header;
    struct button_guide_state button_guide;

    struct list_state boot_list;

    u32 *apps_attr;

    bool autoboot;
    u32 autoboot_item_idx;
    u32 autoboot_delay_sec;

    bool autoboot_countdown;
    u32 autoboot_countdown_sec;
    s32 autoboot_countdown_alarm;

    union osdsys_settings osdsys;

    bool update_file_present;
    bool update_in_progress;
    bool update_complete;
    u32 update_blocks_total;
    u32 update_block_current;
};
