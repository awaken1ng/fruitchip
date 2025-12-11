#pragma once

#include <state.h>

#include <update.h>

void scene_switch_to_update_scanning(struct state *state, enum update_type ty);
void scene_switch_to_update_not_found(enum update_type ty);
void scene_switch_to_update_found(enum update_type ty);
void scene_switch_to_update_checking(enum update_type ty);
void scene_switch_to_update_writing(enum update_type ty, u8 *update, u32 update_size);
void scene_switch_to_update_rebooting(enum update_type ty);
void scene_switch_to_update_complete();

// scanning -> not found -> scanning -> found -> checking -> writing -> rebooting -> complete
