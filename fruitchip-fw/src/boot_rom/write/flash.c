#include "pico/platform/sections.h"

#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>
#include <boot_rom/write/idle.h>
#include <modchip/cmd.h>

void __time_critical_func(handle_write_get_flash_size)(uint8_t w)
{
    static uint32_t response;

    cmd_byte_counter++;

    switch (cmd_byte_counter) {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_FLASH_SIZE, 3)) { goto exit; }
            response = PICO_FLASH_SIZE_BYTES;
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &response, sizeof(response), true);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}
