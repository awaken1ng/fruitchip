#include "pico/platform/sections.h"

#include <modchip/cmd.h>
#include <boot_rom/handler.h>
#include "boot_rom/write/idle.h"
#include "boot_rom/write/disable_next_osdsys_hook.h"
#include "boot_rom/write/get_payload.h"
#include "boot_rom/write/read_app.h"
#include "boot_rom/write/kv.h"

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    switch (w)
    {
        case GET_BYTE(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK, 0): write_handler = prepare_handle_write_disable_next_osdsys_hook; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE1_SIZE,       0): write_handler = prepare_handle_write_get_ee_stage1_size; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2_SIZE,       0): write_handler = prepare_handle_write_get_ee_stage2_size; break;
        case GET_BYTE(MODCHIP_CMD_GET_EE_STAGE2,            0): write_handler = prepare_handle_write_get_ee_stage2; break;
        case GET_BYTE(MODCHIP_CMD_READ_APP,                 0): write_handler = prepare_handle_write_read_apps_partition; break;
        case GET_BYTE(MODCHIP_CMD_KV_GET,                   0): write_handler = prepare_handle_write_kv_get; break;
        case GET_BYTE(MODCHIP_CMD_KV_SET,                   0): write_handler = prepare_handle_write_kv_set; break;
    }
}
