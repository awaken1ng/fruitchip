#include "pico/platform/sections.h"

#include <boot_rom/handler.h>
#include <stdio.h>
#include "idle.h"
#include "disable_next_osdsys_hook.h"
#include "get_payload.h"
#include "read_app.h"
#include <modchip/cmd.h>

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    switch (w)
    {
        case GET_BYTE(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK, 0): write_handler = &handle_write_disable_next_osdsys_hook; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE,       0): write_handler = &handle_write_get_ee_stage1_size; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE,       0): write_handler = &handle_write_get_ee_stage2_size; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2,            0): write_handler = &handle_write_get_ee_stage2; break;
        case GET_BYTE(MODCHIP_CMD_READ_APP,                 0): write_handler = &handle_write_read_apps_partition; break;
    }
}
