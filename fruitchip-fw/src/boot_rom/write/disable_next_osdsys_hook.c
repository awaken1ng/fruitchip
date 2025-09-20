#include "pico/platform/sections.h"

#include "disable_next_osdsys_hook.h"
#include <boot_rom/handler.h>
#include <boot_rom/data_out.h>
#include "idle.h"
#include <modchip/errno.h>
#include <modchip/cmd.h>

static uint8_t counter = 0;

bool disable_next_osdsys_hook = false;

void __time_critical_func(handle_write_disable_next_osdsys_hook)(uint8_t w)
{
    counter++;

    switch (counter) {
        case 1: if (w != GET_BYTE(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK, 3)) { goto exit; }
            disable_next_osdsys_hook = true;
            boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
            counter = 0;
    }
}

void __time_critical_func(prepare_handle_write_disable_next_osdsys_hook)(uint8_t w)
{
    counter = 0;
    write_handler = handle_write_disable_next_osdsys_hook;
    write_handler(w);
}
