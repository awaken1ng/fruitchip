#include "pico/platform/sections.h"

#include <modchip/cmd.h>
#include <boot_rom/handler.h>
#include "boot_rom/write/idle.h"
#include "boot_rom/write/configuration.h"
#include "boot_rom/write/patch_control.h"
#include "boot_rom/write/get_payload.h"
#include "boot_rom/write/read_app.h"
#include "boot_rom/write/kv.h"
#include "boot_rom/write/version.h"
#include "boot_rom/write/flash.h"
#include "boot_rom/write/test.h"

void __time_critical_func(handle_write_cmd_group0)(uint8_t w)
{
    cmd_byte_counter++; // 2

    // assert that cmd belongs to group0 (2nd byte), and extract 3rd byte to match on
    #define assert_and_case(v) static_assert((uint8_t)(v >> 8) == MODCHIP_CMD_GROUP0); case ((uint8_t)(v >> 16))

    switch (w)
    {
        assert_and_case(MODCHIP_CMD_DISABLE_NEXT_OSDSYS_HOOK): write_handler = handle_write_disable_next_osdsys_hook; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE1_SIZE): write_handler = handle_write_get_ee_stage1_size; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE2_SIZE): write_handler = handle_write_get_ee_stage2_size; break;
        assert_and_case(MODCHIP_CMD_GET_EE_STAGE2): write_handler = handle_write_get_ee_stage2; break;
        assert_and_case(MODCHIP_CMD_READ_EE_STAGE3): write_handler = handle_write_read_ee_stage3; break;
        assert_and_case(MODCHIP_CMD_READ_APP): write_handler = handle_write_read_apps_partition; break;
        assert_and_case(MODCHIP_CMD_KV_GET): write_handler = handle_write_kv_get; break;
        assert_and_case(MODCHIP_CMD_KV_SET): write_handler = handle_write_kv_set; break;
        assert_and_case(MODCHIP_CMD_FW_GIT_REV): write_handler = handle_write_fw_git_rev; break;
        assert_and_case(MODCHIP_CMD_BOOTLOADER_GIT_REV): write_handler = handle_write_bootloader_git_rev; break;
        assert_and_case(MODCHIP_CMD_GET_PARTITION_SIZE): write_handler = handle_write_get_partition_size; break;
        assert_and_case(MODCHIP_CMD_GET_FLASH_SECTOR_SIZE): write_handler = handle_write_get_flash_sector_size; break;
        assert_and_case(MODCHIP_CMD_WRITE_LOCK): write_handler = handle_write_write_lock; break;
        assert_and_case(MODCHIP_CMD_ERASE_FLASH_SECTOR): write_handler = handle_write_erase_flash_sector; break;
        assert_and_case(MODCHIP_CMD_WRITE_FLASH_SECTOR): write_handler = handle_write_write_flash_sector; break;
        assert_and_case(MODCHIP_CMD_REBOOT): write_handler = handle_write_reboot; break;
        assert_and_case(MODCHIP_CMD_PIN_CONFIG): write_handler = handle_write_pin_config; break;
        default: write_handler = handle_write_idle; break;
    }

    #undef assert_and_case
}

void __time_critical_func(handle_write_cmd_group1)(uint8_t w)
{
    cmd_byte_counter++; // 2

    // assert that cmd belongs to group1 (2nd byte), and extract 3rd byte to match on
    #define assert_and_case(v) static_assert((uint8_t)(v >> 8) == MODCHIP_CMD_GROUP1); case ((uint8_t)(v >> 16))

    switch (w)
    {
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_NO_DATA_NO_CRC): write_handler = handle_write_test_data_out_with_status_no_data_no_crc; break;
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_NO_CRC): write_handler = handle_write_test_data_out_with_status_with_data_no_crc; break;
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_WITH_CRC): write_handler = handle_write_test_data_out_with_status_with_data_with_crc; break;
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_WITH_CRC): write_handler = handle_write_test_data_out_no_status_with_data_with_crc; break;
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_NO_CRC): write_handler = handle_write_test_data_out_no_status_with_data_no_crc; break;
        assert_and_case(MODCHIP_CMD_TEST_DATA_OUT_BUSY): write_handler = handle_write_test_data_out_busy_code; break;
        default: write_handler = handle_write_idle; break;
    }

    #undef assert_and_case

}

void __time_critical_func(handle_write_cmd)(uint8_t w)
{
    cmd_byte_counter++; // 1

    switch (w)
    {
        case MODCHIP_CMD_GROUP0:
            write_handler = handle_write_cmd_group0;
            break;
        case MODCHIP_CMD_GROUP1:
            write_handler = handle_write_cmd_group1;
            break;
        default:
            write_handler = handle_write_idle;
            break;
    }
}


void __time_critical_func(handle_write_idle)(uint8_t w)
{
    cmd_byte_counter = 0;

    switch (w)
    {
        case MODCHIP_CMD_PREFIX:
            write_handler = handle_write_cmd;
            break;
    }
}
