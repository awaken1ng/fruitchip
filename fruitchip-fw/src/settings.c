#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pico/multicore.h>

#include "wear_leveling.h"
#include "panic.h"

#define SETTINGS_VERSION_MAGIC (0x73de0002)

typedef struct {
    uint32_t version_magic;
    bool menu_autoboot;
    uint16_t menu_autoboot_delay_sec;
    uint8_t menu_autoboot_item_idx;
    uint8_t menu_osdsys_options;
} settings_t;

static settings_t settings;

static void settings_reset(void)
{
    memset(&settings, 0, sizeof(settings));
    settings.version_magic = SETTINGS_VERSION_MAGIC;
    settings.menu_autoboot = true;
    settings.menu_autoboot_delay_sec = 10;
    settings.menu_autoboot_item_idx = 0; // OSDSYS

    if (wear_leveling_write(0, &settings, sizeof(settings)) == WEAR_LEVELING_FAILED)
        panic_with_led(RGB_ERR_SETTINGS, "settings reset failed");
}

void settings_init(void)
{
    if (wear_leveling_init() == WEAR_LEVELING_FAILED)
    {
        printf("wear leveling init failed, resetting settings\n");
        settings_reset();

        if (wear_leveling_init() == WEAR_LEVELING_FAILED)
            panic_with_led(RGB_ERR_SETTINGS, "wear leveling init failed");
    }

    wear_leveling_read(0, &settings, sizeof(settings));

    if (settings.version_magic != SETTINGS_VERSION_MAGIC)
    {
        printf("version magic mismatch, resetting settings\n");
        settings_reset();
    }
}

static void settings_update_part(void *settings_ptr, uint32_t sz)
{
    if (multicore_lockout_victim_is_initialized(1))
       multicore_lockout_start_blocking();

    wear_leveling_write((uint8_t*)settings_ptr - (uint8_t*)&settings, settings_ptr, sz);

    if (multicore_lockout_victim_is_initialized(1))
        multicore_lockout_end_blocking();
}

#define settings_update_field(field) settings_update_part(&settings.field, sizeof(settings.field))

// region: menu_autoboot

bool settings_get_menu_autoboot()
{
    return settings.menu_autoboot;
}

void settings_set_menu_autoboot(bool menu_autoboot)
{
    if (settings.menu_autoboot == menu_autoboot) return;
    settings.menu_autoboot = menu_autoboot;
    settings_update_field(menu_autoboot);
}

// endregion: menu_autoboot

// region: menu_autoboot_delay_sec

uint16_t settings_get_menu_autoboot_delay_sec()
{
    return settings.menu_autoboot_delay_sec;
}

void settings_set_menu_autoboot_delay_sec(uint16_t menu_autoboot_delay_sec)
{
    if (settings.menu_autoboot_delay_sec == menu_autoboot_delay_sec) return;
    settings.menu_autoboot_delay_sec = menu_autoboot_delay_sec;
    settings_update_field(menu_autoboot_delay_sec);
}

// endregion: menu_autoboot_delay_sec

// region: menu_autoboot_item_idx

uint16_t settings_get_menu_autoboot_item_idx()
{
    return settings.menu_autoboot_item_idx;
}

void settings_set_menu_autoboot_item_idx(uint16_t menu_autoboot_item_idx)
{
    if (settings.menu_autoboot_item_idx == menu_autoboot_item_idx) return;
    settings.menu_autoboot_item_idx = menu_autoboot_item_idx;
    settings_update_field(menu_autoboot_item_idx);
}

// endregion: menu_autoboot_item_idx


// region: menu_osdsys_options

uint8_t settings_get_menu_osdsys_options()
{
    return settings.menu_osdsys_options;
}

void settings_set_menu_osdsys_options(uint8_t menu_osdsys_options)
{
    if (settings.menu_osdsys_options == menu_osdsys_options) return;
    settings.menu_osdsys_options = menu_osdsys_options;
    settings_update_field(menu_osdsys_options);
}

// endregion: menu_osdsys_options
