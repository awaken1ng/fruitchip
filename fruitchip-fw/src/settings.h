#pragma once

#include <stdint.h>

void settings_init(void);

bool settings_get_menu_autoboot();
void settings_set_menu_autoboot(bool menu_autoboot);

uint16_t settings_get_menu_autoboot_delay_sec();
void settings_set_menu_autoboot_delay_sec(uint16_t menu_autoboot_delay_sec);

uint16_t settings_get_menu_autoboot_item_idx();
void settings_set_menu_autoboot_item_idx(uint16_t menu_autoboot_item_idx);

uint8_t settings_get_menu_osdsys_options();
void settings_set_menu_osdsys_options(uint8_t menu_osdsys_options);
