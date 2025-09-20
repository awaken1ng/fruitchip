#include "pico/platform/sections.h"

#include <modchip/cmd.h>
#include <boot_rom/handler.h>
#include "boot_rom/write/idle.h"
#include "boot_rom/write/disable_next_osdsys_hook.h"
#include "boot_rom/write/get_payload.h"
#include "boot_rom/write/read_app.h"
#include "boot_rom/write/kv.h"
#include "boot_rom/write/version.h"
#include "boot_rom/write/flash.h"

void __time_critical_func(handle_write_cmd_group0)(uint8_t w)
{
    // assert that cmd belongs to group0 (2nd byte), and extract 3rd byte to match on
    #define assert_and_case(v) static_assert((uint8_t)(v >> 8) == MODCHIP_CMD_GROUP0); case ((uint8_t)(v >> 16))

    switch (w)
    {
        assert_and_case(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK): write_handler = prepare_handle_write_disable_next_osdsys_hook; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE1_SIZE): write_handler = prepare_handle_write_get_ee_stage1_size; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE2_SIZE): write_handler = prepare_handle_write_get_ee_stage2_size; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE2): write_handler = prepare_handle_write_get_ee_stage2; break;
        assert_and_case(MODCHIP_CMD_READ_APP): write_handler = prepare_handle_write_read_apps_partition; break;
        assert_and_case(MODCHIP_CMD_KV_GET): write_handler = prepare_handle_write_kv_get; break;
        assert_and_case(MODCHIP_CMD_KV_SET): write_handler = prepare_handle_write_kv_set; break;
        assert_and_case(MODCHIP_CMD_GIT_REV): write_handler = prepare_handle_write_git_rev; break;
        assert_and_case(MODCHIP_CMD_GET_FLASH_SIZE): write_handler = prepare_handle_write_get_flash_size; break;
        default: write_handler = handle_write_idle; break;
    }

    #undef assert_and_case
}

void __time_critical_func(handle_write_cmd)(uint8_t w)
{
    switch (w)
    {
        case MODCHIP_CMD_GROUP0:
            write_handler = handle_write_cmd_group0;
            break;
        default:
            write_handler = handle_write_idle;
            break;
    }
}

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    switch (w)
    {
        case MODCHIP_CMD_PREFIX:
            write_handler = handle_write_cmd;
            break;
    }
}
