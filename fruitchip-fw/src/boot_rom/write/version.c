#include "pico/platform/sections.h"

#include "boot_rom/handler.h"
#include "boot_rom/write/idle.h"
#include "boot_rom/write/version.h"
#include "boot_rom/data_out.h"
#include <modchip/cmd.h>
#include "../../version.h"

void __time_critical_func(handle_write_git_rev)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GIT_REV, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, GIT_REV, 8, false);
exit:
        [[fallthrough]];
        default:
            write_handler = handle_write_idle;
    }
}
