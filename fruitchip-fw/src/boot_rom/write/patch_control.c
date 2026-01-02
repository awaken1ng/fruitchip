#include <pico/platform/sections.h>

#include <modchip/errno.h>
#include <modchip/cmd.h>

#include <boot_rom/write/patch_control.h>
#include <boot_rom/write/idle.h>
#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>

bool disable_next_osdsys_hook = false;
bool disable_next_iopboot_hook = true;

void __time_critical_func(handle_write_disable_next_osdsys_hook)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK, 3)) { goto exit; }
            disable_next_osdsys_hook = true;
            boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}
