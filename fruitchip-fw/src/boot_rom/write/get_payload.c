#include "pico/platform/sections.h"

#include "get_payload.h"
#include <boot_rom/handler.h>
#include <boot_rom/data_out.h>
#include <boot_rom/loader.h>
#include <modchip/errno.h>
#include <modchip/cmd.h>
#include "idle.h"

void __time_critical_func(handle_write_get_ee_stage1_size)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_1_SIZE, sizeof(LOADER_EE_STAGE_1_SIZE), false);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_get_ee_stage2_size)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_2_SIZE, sizeof(LOADER_EE_STAGE_2_SIZE), false);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_get_ee_stage2)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, LOADER_EE_STAGE_2, LOADER_EE_STAGE_2_SIZE, true);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

static uint32_t read_offset = 0;
static uint32_t read_offset_xor = 0;
static uint32_t read_size = 0;
static uint32_t read_size_xor = 0;
static bool with_crc = 0;

static void __time_critical_func(read_ee_stage3)(uint8_t w)
{
    // cmd_byte_counter = 21

    uint8_t with_crc_xor = w;

    bool is_valid_read_offset = (read_offset ^ read_offset_xor) == 0xFFFFFFFF && LOADER_EE_STAGE_3_SIZE  > read_offset;
    bool is_valid_read_size = (read_size ^ read_size_xor) == 0xFFFFFFFF;
    bool is_valid_with_crc = (with_crc ^ with_crc_xor) == 0xFF;
    bool is_valid = is_valid_read_offset && is_valid_read_size && is_valid_with_crc;

    if (is_valid)
    {
        const uint8_t *addr = LOADER_EE_STAGE_3 + read_offset;
        boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, addr, read_size, with_crc);
    }
    else
    {
        boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
    }

    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_read_ee_stage3)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_READ_EE_STAGE3, 3)) { goto exit; } break;

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

        case 20:
            with_crc = w;
            write_handler = &read_ee_stage3;
            break;

exit:
        default:
            write_handler = &handle_write_idle;
    }
}
