#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "errno.h"

#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "utility.h"

static u8 *apps_read_index()
{
    int fd = open("fwfs:" FWFS_MODE_DATA_STR "\0", 0);
    if (fd >= 0)
    {
        __off_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        print_combined("apps list length: %ld\n", len);

        u8 *apps_index = malloc(len);
        ssize_t ret = read(fd, apps_index, len);
        close(fd);

        if (ret != len)
        {
            free(apps_index);
            return NULL;
        }

        return apps_index;
    }

    return NULL;
}

static u32 app_read_attributes(u8 app_idx)
{
    char path[] = { 'f', 'w', 'f', 's', ':', FWFS_MODE_ATTR_CHAR, app_idx };

    u32 attr = 0;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -ENOENT;

    ssize_t ret = read(fd, &attr, sizeof(attr));
    close(fd);

    if (ret != sizeof(attr))
        return -EIO;

    print_combined("app_idx %i attr %i\n", app_idx, attr);
    return attr;
}

static u32 app_read_data(u8 app_idx)
{
    char path[] = { 'f', 'w', 'f', 's', ':', FWFS_MODE_DATA_CHAR, app_idx };

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -ENOENT;

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    print_combined("app_idx %i size %i\n", app_idx, size);

    u8 *uf2 = malloc(size);
    int bytes_read = read(fd, (void *)uf2, size);
    if (bytes_read != size)
    {
        return -EIO;
    }

    free(uf2);
    close(fd);

    return 0;
}

bool test_menu()
{
    bool failed = false;

    u32 v;
    failed |= !modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT, &v);
    if (failed) print_combined("%s: setting %i get failed\n", __func__, MODCHIP_SETTINGS_MENU_AUTOBOOT);
    failed |= !modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY, &v);
    if (failed) print_combined("%s: setting %i get failed\n", __func__, MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY);

    u8 *apps_index = apps_read_index();
    failed |= apps_index == NULL;
    if (failed) print_combined("%s: index read failed\n", __func__);

    if (apps_index)
    {
        u8 *ptr = apps_index;
        u8 apps_count = *ptr++;

        for (u8 idx = 0; idx < apps_count; idx++)
        {
            u8 app_idx = idx + 1;
            u32 ret = app_read_attributes(app_idx);
            failed |= (ret < 0);
            if (failed) print_combined("%s: attr %i read failed, ret %i\n", __func__, app_idx, ret);
        }

        for (u8 idx = 0; idx < apps_count; idx++)
        {
            u8 app_idx = idx + 1;
            u32 ret = app_read_data(app_idx);
            failed |= (ret < 0);
            if (failed) print_combined("%s: data %i read failed %i, ret\n", __func__, app_idx, ret);
        }

        free(apps_index);
    }

    failed |= !modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX, &v);
    if (failed) print_combined("%s: setting %i get failed\n", __func__, MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX);

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
