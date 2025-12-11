#pragma once

#include <stdbool.h>

#define FW_UPDATE_PATH "fruitchip/update/fw.uf2"
#define APPS_UPDATE_PATH "fruitchip/update/apps.uf2"

enum update_type {
    UPDATE_TYPE_FW,
    UPDATE_TYPE_APPS,
};

int update_file_open(enum update_type ty);
bool update_file_is_present(enum update_type ty);
