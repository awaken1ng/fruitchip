#pragma once

#include <stdbool.h>

bool version_init_firmware();
bool version_init_bootloader();
const char *version_get_menu();
const char *version_get_firmware();
const char *version_get_bootloader();
