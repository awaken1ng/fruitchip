#include "modchip/fwfs.h"
#include "modchip/apps.h"
#include "modchip/apps.h"

#include "utility.h"

static bool test_apps_read_unaligned()
{
    s32 offset = 0;
    u8 app_idx = 0;
    u8 buffer[16];

    bool failed = false;

    for (int size = 1; size <= 8; size++)
    {
        s32 r = modchip_apps_read(offset, size, app_idx, buffer, true);
        failed |= (r != 0);
        if (failed)
            print_combined("%s: offset %i size %i app_idx %i result %i\n", __func__, offset, size, app_idx, r);
    }

    return !failed;
}

static bool test_apps_read_uncached()
{
    s32 offset = 16 * 1024;
    s32 size = 16 * 1024;
    u8 app_idx = 1;
    u8 buffer[16 * 1024];

    bool failed = false;

    // XIP cache is 16 KiB, push out existing cache
    s32 r = modchip_apps_read(offset, size, app_idx, buffer, true);
    failed |= (r != 0);
    if (failed)
        print_combined("%s: offset %i size %i app_idx %i result %i\n", __func__, offset, size, app_idx, r);

    size = 4;
    for (int i = 0; i < 4; i++)
    {
        offset = i * 1024;
        r = modchip_apps_read(offset, size, app_idx, buffer, true);
        failed |= (r != 0);
        if (failed)
            print_combined("%s: offset %i size %i app_idx %i result %i\n", __func__, offset, size, app_idx, r);
    }

    return !failed;
}

bool test_apps()
{
    bool failed = false;

    failed |= !test_apps_read_unaligned();
    failed |= !test_apps_read_uncached();

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
