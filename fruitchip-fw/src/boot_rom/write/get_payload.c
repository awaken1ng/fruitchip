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
