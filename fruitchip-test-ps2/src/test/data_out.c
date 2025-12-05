#include <string.h>

#include "modchip/apps.h"
#include "modchip/flash.h"
#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "utility.h"

static uint8_t placeholder[15] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static bool test_data_out_with_status_no_data_no_crc()
{
    printf("%s\n", __func__);

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_NO_DATA_NO_CRC);
    u32 r = modchip_peek_u32();
    switch (r) {
        case MODCHIP_CMD_RESULT_OK:
            return true;
        case MODCHIP_CMD_RESULT_ERR:
            print_combined("%s: error\n", __func__);
            return false;
        default:
            print_combined("%s: did not respond\n", __func__);
            return false;
    }
}

static bool test_data_out_with_status_with_data_no_crc()
{
    printf("%s\n", __func__);

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_NO_CRC);
    u32 r = modchip_peek_u32();

    u8 resp[16];

    switch (r) {
        case MODCHIP_CMD_RESULT_OK:
            modchip_peek_n(resp, 15);
            bool is_eq = memcmp(resp, placeholder, 15) == 0;
            if (!is_eq)
            {
                print_combined("%s: response did not match\n", __func__);
            }

            return is_eq;
        case MODCHIP_CMD_RESULT_ERR:
            print_combined("%s: error\n", __func__);
            return false;
        default:
            print_combined("%s: did not respond\n", __func__);
            return false;
    }
}

static bool test_data_out_with_status_with_data_with_crc()
{
    printf("%s\n", __func__);

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_WITH_CRC);
    u32 r = modchip_peek_u32();

    u8 resp[16];

    switch (r) {
        case MODCHIP_CMD_RESULT_OK:
            modchip_peek_n(resp, 15);
            bool is_eq = memcmp(resp, placeholder, sizeof(placeholder)) == 0;
            if (!is_eq)
            {
                print_combined("%s: response did not match\n", __func__);
            }

            u32 crc_expected = modchip_peek_u32();
            u32 crc_actual = crc32(resp, sizeof(placeholder));
            bool crc_matches = crc_expected == crc_actual;
            if (!crc_matches)
            {
                print_combined("%s: crc did not match\n", __func__);
            }

            return is_eq && crc_matches;
        case MODCHIP_CMD_RESULT_ERR:
            print_combined("%s: error\n", __func__);
            return false;
        default:
            print_combined("%s: did not respond\n", __func__);
            return false;
    }
}

static bool test_data_out_no_status_with_data_with_crc()
{
    printf("%s\n", __func__);

    u8 resp[16];

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_WITH_CRC);
    modchip_peek_n(resp, 15);
    bool is_eq = memcmp(resp, placeholder, 15) == 0;
    if (!is_eq)
    {
        print_combined("%s: response did not match\n", __func__);
    }

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32(resp, sizeof(placeholder));
    bool crc_matches = crc_expected == crc_actual;
    if (!crc_matches)
    {
        print_combined("%s: crc did not match\n", __func__);
    }

    return is_eq && crc_matches;
}

static bool test_data_out_no_status_with_data_no_crc()
{
    printf("%s\n", __func__);

    u8 resp[16];

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_NO_CRC);
    modchip_peek_n(resp, 15);
    bool is_eq = memcmp(resp, placeholder, 15) == 0;
    if (!is_eq)
    {
        print_combined("%s: response did not match\n", __func__);
    }

    return is_eq;
}

static bool test_busy_code()
{
    printf("%s\n", __func__);

    modchip_poke_u32(MODCHIP_CMD_TEST_DATA_OUT_BUSY);

    int busy = 0;

    while (true)
    {
        u32 r = modchip_peek_u32();
        switch (r)
        {
            case MODCHIP_CMD_RESULT_BUSY:
            {
                busy += 1;

                if (busy > 32)
                {
                    print_combined("%s: timed out\n", __func__);
                }
                else
                {
                    continue;
                }
            }
            case MODCHIP_CMD_RESULT_OK:
                return true;
            case MODCHIP_CMD_RESULT_ERR:
                print_combined("%s: error\n", __func__);
                return false;
            default:
                print_combined("%s: did not respond\n", __func__);
                return false;
        }
    }
}

bool test_data_out()
{
    bool failed = false;

    failed |= !test_data_out_with_status_no_data_no_crc();
    failed |= !test_data_out_with_status_with_data_no_crc();
    failed |= !test_data_out_with_status_with_data_with_crc();
    failed |= !test_data_out_no_status_with_data_no_crc();
    failed |= !test_data_out_no_status_with_data_with_crc();
    failed |= !test_busy_code();

    if (!failed)
    {
        print_combined("%s: pass\n", __func__);
    }
    else
    {
        print_combined("%s: fail\n", __func__);
    }

    return !failed;
}
