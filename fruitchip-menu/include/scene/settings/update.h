#pragma once

#include <state.h>

#include <update.h>

void scene_switch_to_update_scanning(struct state *state);
void scene_switch_to_update_not_found();
void scene_switch_to_update_found();
void scene_switch_to_update_checking();
void scene_switch_to_update_writing(u8 *update, u32 update_size);
void scene_switch_to_update_rebooting();
void scene_switch_to_update_complete();

// scanning -> not found -> scanning -> found -> checking -> writing -> rebooting -> complete
