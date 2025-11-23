#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <tamtypes.h>

#include "modchip/apps.h"
#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "wstring.h"
#include "constants.h"
#include "components/list.h"
#include "apps.h"
#include "state.h"
#include "utils.h"

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

    return attr;
}

static u8 *apps_read_index()
{
    int fd = open("fwfs:" FWFS_MODE_DATA_STR "\0", 0);
    if (fd >= 0)
    {
        __off_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        print_debug("apps list length: %ld\n", len);

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

bool apps_list_populate(struct state *state)
{
    bool ret = false;

    list_clear(&state->boot_list);
    array_u32_clear(state->boot_list_attr);

    state->boot_list.hilite_idx = BOOT_ITEM_OSDSYS;
    state->boot_list.start_item_idx = 0;
    state->boot_list.max_items = MAX_LIST_ITEMS_ON_SCREEN;

    array_u32_init(state->boot_list_attr);

    struct list_item list_item;
    list_item.left_text = wstring_new_static(L"OSDSYS");
    apps_list_push_item(state, list_item, MODCHIP_APPS_ATTR_DISABLE_NEXT_OSDSYS_HOOK | MODCHIP_APPS_ATTR_OSDSYS);

    u8 *apps_index = apps_read_index();
    if (!apps_index)
        goto exit;

    u8 *ptr = apps_index;
    u8 apps_count = *ptr++;
    print_debug("apps count %i\n", apps_count);

    for (u8 idx = 0; idx < apps_count; idx++)
    {
        u8 name_len = *ptr++;
        list_item.left_text = wstring_new_copied_str((char *)ptr, name_len);

        u8 app_idx = idx + 1;
        u32 attr = app_read_attributes(app_idx);
        if (attr < 0)
        {
            print_debug("failed to read attributes for app idx %i: %i", app_idx, attr);
            attr = 0;
        }
        apps_list_push_item(state, list_item, attr);

        ptr += name_len;
    }

    free(apps_index);

    int res = modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX, &state->autoboot_item_idx);
    print_debug("MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX %u ret %i\n", state->autoboot_item_idx, res);
    if (res && state->autoboot_item_idx < list_len(&state->boot_list))
        state->boot_list.hilite_idx = state->autoboot_item_idx;

    ret = true;

exit:
    return ret;
}

u32 apps_list_push_item(struct state *state, list_item_t item, u32 attr)
{
    u32 idx = list_push_item(&state->boot_list, item);
    array_u32_push_back(state->boot_list_attr, attr);
    return idx;
}

void apps_list_remove_item(struct state *state, u32 idx)
{
    list_remove_item(&state->boot_list, idx);
    array_u32_remove_v(state->boot_list_attr, idx, idx + 1);
}

bool apps_attr_is_configurable(u32 attr)
{
    bool ret = false;

    if (attr & MODCHIP_APPS_ATTR_OSDSYS)
        ret |= true;

    return ret;
}
