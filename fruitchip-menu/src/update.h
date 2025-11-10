#pragma once

#include "state.h"

#define FW_UPDATE_PATH "fruitchip/update/fw.uf2"
#define APPS_UPDATE_PATH "fruitchip/update/apps.uf2"

bool update_is_fw_update_file_present();
bool update_is_apps_update_file_present();
void update_start_fw_update(struct state *state);
void update_start_apps_update(struct state *state);
