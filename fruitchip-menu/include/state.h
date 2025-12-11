#pragma once

#include <wchar.h>

#include <tamtypes.h>
#include <gsKit.h>

#include "mlib/m-array.h"

#include "input/pad.h"
#include "components/list.h"
#include "components/button_guide.h"
#include "update.h"

ARRAY_DEF(array_u32, u32)

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

    list_state_t boot_list;
    array_u32_t boot_list_attr;

    bool mass_event_received;
    u32 rescue_item_idx;

    bool autoboot;
    u32 autoboot_item_idx;
    u32 autoboot_delay_sec;

    bool autoboot_countdown;
    u32 autoboot_countdown_sec;
    s32 autoboot_countdown_alarm;

    union osdsys_settings osdsys;

    enum update_type update_type;
};
