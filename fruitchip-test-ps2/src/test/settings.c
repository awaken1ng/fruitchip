#include "modchip/settings.h"

#include "utility.h"

bool test_settings()
{
    bool failed = false;

    u32 keys[MODCHIP_SETTINGS_KEYS_TOTAL] = {
        MODCHIP_SETTINGS_MENU_AUTOBOOT,
        MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY,
        MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX,
        MODCHIP_SETTINGS_MENU_OSDSYS_SETTINGS,
    };

    for (int i = 0; i < 4; i++)
    {
        u32 value;
        failed |= !modchip_settings_get(keys[i], &value);
        if (failed)
        {
            print_combined("%s: setting %i get failed %i\n", __func__, i, failed);
            break;
        }
        else
        {
            failed |= !modchip_settings_set(keys[i], value);
            if (failed)
            {
                print_combined("%s: setting %i set failed %i\n", __func__, i, failed);
                break;
            }
        }
    }

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
