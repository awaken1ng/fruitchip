#include "pico/platform/sections.h"

#include "read_app.h"
#include "apps.h"
#include <modchip/errno.h>
#include <modchip/cmd.h>
#include <boot_rom/handler.h>
#include "idle.h"

void __time_critical_func(handle_write_read_apps_partition)(uint8_t w)
{
    static uint8_t counter = 0;

    static uint32_t read_offset = 0;
    static uint32_t read_offset_xor = 0;
    static uint32_t read_size = 0;
    static uint32_t read_size_xor = 0;
    static uint8_t app_idx = 0;

    counter += 1;

    switch (counter) {
        case 1: if (w != GET_BYTE(MODCHIP_CMD_READ_APP, 1)) { goto exit; } break;
        case 2: if (w != GET_BYTE(MODCHIP_CMD_READ_APP, 2)) { goto exit; } break;
        case 3: if (w != GET_BYTE(MODCHIP_CMD_READ_APP, 3)) { goto exit; } break;

        case 4: read_offset = w; break;
        case 5: read_offset |= w << 8; break;
        case 6: read_offset |= w << 16; break;
        case 7: read_offset |= w << 24; break;

        case 8: read_offset_xor = w; break;
        case 9: read_offset_xor |= w << 8; break;
        case 10: read_offset_xor |= w << 16; break;
        case 11: read_offset_xor |= w << 24; break;

        case 12: read_size = w; break;
        case 13: read_size |= w << 8; break;
        case 14: read_size |= w << 16; break;
        case 15: read_size |= w << 24; break;

        case 16: read_size_xor = w; break;
        case 17: read_size_xor |= w << 8; break;
        case 18: read_size_xor |= w << 16; break;
        case 19: read_size_xor |= w << 24; break;

        case 20: app_idx = w; break;
        case 21:
        {
            uint8_t app_idx_xor = w;

            bool is_valid_read_offset = (read_offset ^ read_offset_xor) == 0xFFFFFFFF && apps_partition_entry_length(w)  > read_offset;
            bool is_valid_read_size = (read_size ^ read_size_xor) == 0xFFFFFFFF;
            bool is_valid_app_idx = (app_idx ^ app_idx_xor) == 0xFF && apps_partition_entries_count() > w;

            if (is_valid_read_offset && is_valid_read_size && is_valid_app_idx)
            {
                uint8_t *addr = apps_partition_entry_addr(w) + read_offset;
                boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, addr, read_size, false);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
            counter = 0;
    }
}
