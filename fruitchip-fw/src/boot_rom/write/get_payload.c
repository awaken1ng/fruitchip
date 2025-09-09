#include "pico/platform/sections.h"

#include "get_payload.h"
#include <boot_rom/handler.h>
#include <boot_rom/data_out.h>
#include <boot_rom/loader.h>
#include <modchip/errno.h>
#include <modchip/cmd.h>
#include "idle.h"

static uint8_t counter = 0;

void __time_critical_func(handle_write_get_ee_stage1_size)(uint8_t w)
{
    counter++;

    if      (counter == 1 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE, 1)) {}
    else if (counter == 2 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE, 2)) {}
    else if (counter == 3 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE, 3))
    {
        boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_1_SIZE, sizeof(LOADER_EE_STAGE_1_SIZE), false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage1)(uint8_t w)
{
    counter++;

    if      (counter == 1 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1, 1)) {}
    else if (counter == 2 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1, 2)) {}
    else if (counter == 3 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1, 3))
    {
        boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_1, LOADER_EE_STAGE_1_SIZE, false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage2_size)(uint8_t w)
{
    counter++;

    if      (counter == 1 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE, 1)) {}
    else if (counter == 2 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE, 2)) {}
    else if (counter == 3 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE, 3))
    {
        boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_2_SIZE, sizeof(LOADER_EE_STAGE_2_SIZE), false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage2)(uint8_t w)
{
    counter++;

    if      (counter == 1 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2, 1)) {}
    else if (counter == 2 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2, 2)) {}
    else if (counter == 3 && w == GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2, 3))
    {
        boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_2, LOADER_EE_STAGE_2_SIZE, false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}
