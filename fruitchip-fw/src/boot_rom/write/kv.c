#include "modchip/cmd.h"
#include "modchip/errno.h"
#include "boot_rom/handler.h"
#include "boot_rom/data_out.h"
#include "boot_rom/write/idle.h"
#include "boot_rom/write/kv.h"
#include "settings.h"

void __time_critical_func(handle_write_kv_get)(uint8_t w)
{
    static uint16_t idx = 0;
    static uint16_t idx_xor = 0;

    static uint32_t response;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_KV_GET, 3)) { goto exit; }

        case 4: idx = w; break;
        case 5: idx |= w << 8; break;

        case 6: idx_xor = w; break;
        case 7: idx_xor |= w << 8;
        {
            bool is_valid_idx = (idx ^ idx_xor) == 0xFFFF && (idx < MODCHIP_SETTINGS_KEYS_TOTAL);
            bool is_valid = is_valid_idx;

            if (is_valid)
            {
                switch (idx)
                {
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT: response = settings_get_menu_autoboot(); break;
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY: response = settings_get_menu_autoboot_delay_sec(); break;
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX: response = settings_get_menu_autoboot_item_idx(); break;
                    case MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS: response = settings_get_menu_osdsys_options(); break;
                }

                boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &response, sizeof(response), true);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }

exit:
        [[fallthrough]];
        default:
            write_handler = handle_write_idle;
    }
}

void __time_critical_func(handle_write_kv_set)(uint8_t w)
{
    static uint16_t idx = 0;
    static uint16_t idx_xor = 0;
    static uint32_t value = 0;
    static uint32_t value_xor = 0;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_KV_SET, 3)) { goto exit; }

        case 4: idx = w; break;
        case 5: idx |= w << 8; break;

        case 6: idx_xor = w; break;
        case 7: idx_xor |= w << 8; break;

        case 8: value = w; break;
        case 9: value |= w << 8; break;
        case 10: value |= w << 16; break;
        case 11: value |= w << 24; break;

        case 12: value_xor = w; break;
        case 13: value_xor |= w << 8; break;
        case 14: value_xor |= w << 16; break;
        case 15: value_xor |= w << 24;
        {
            bool is_valid_idx = (idx ^ idx_xor) == 0xFFFF && (idx < MODCHIP_SETTINGS_KEYS_TOTAL);
            bool is_valid_read_offset = (value ^ value_xor) == 0xFFFFFFFF;
            bool is_valid = is_valid_idx && is_valid_read_offset;

            if (is_valid)
            {
                boot_rom_data_out_start_busy_code();

                switch (idx)
                {
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT: settings_set_menu_autoboot(value); break;
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY: settings_set_menu_autoboot_delay_sec(value); break;
                    case MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX: settings_set_menu_autoboot_item_idx(value); break;
                    case MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS: settings_set_menu_osdsys_options(value); break;
                }

                boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_OK);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }

exit:
        [[fallthrough]];
        default:
            write_handler = handle_write_idle;
    }
}
