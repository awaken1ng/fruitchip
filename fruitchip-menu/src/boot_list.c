#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <tamtypes.h>

#include "modchip/apps.h"
#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "wstring.h"
#include "constants.h"
#include "components/list.h"
#include "state.h"
#include "boot_list.h"

static u32 app_read_attributes(u8 app_idx)
{
    char path[] = { 'f', 'w', 'f', 's', ':', FWFS_MODE_ATTR_CHAR, app_idx };

    u32 attr = 0;
    int fd = open(path, O_RDONLY);
    read(fd, &attr, sizeof(attr));
    close(fd);

    return attr;
}

void apps_list_populate(struct state *state)
{
    state->boot_list.hilite_idx = BOOT_ITEM_OSDSYS;
    state->boot_list.start_item_idx = 0;
    state->boot_list.max_items = MAX_LIST_ITEMS_ON_SCREEN;

    while (state->boot_list.items_count) list_pop_item(&state->boot_list);

    struct list_item list_item = { .left_text = wstring_new_static(L"OSDSYS") };
    list_push_item(&state->boot_list, &list_item);

    int fd = open("fwfs:" FWFS_MODE_DATA_STR "\0", 0);
    if (fd >= 0)
    {
        __off_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        printf("Apps list length: %ld\n", len);

        u8 *apps_index = malloc(len);
        read(fd, apps_index, len);
        close(fd);

        u8 *ptr = apps_index;
        u8 apps_count = *ptr++;

        free(state->apps_attr);
        state->apps_attr = malloc(sizeof(*state->apps_attr) * (apps_count + 1));
        state->apps_attr[BOOT_ITEM_OSDSYS] = MODCHIP_APPS_ATTR_DISABLE_NEXT_OSDSYS_HOOK | MODCHIP_APPS_ATTR_OSDSYS;

        for (u8 idx = 0; idx < apps_count; idx++)
        {
            u8 name_len = *ptr++;
            list_item.left_text = wstring_new_copied_str((char *)ptr, name_len);
            ptr += name_len;
            list_push_item(&state->boot_list, &list_item);

            u8 app_idx = idx + 1;
            state->apps_attr[app_idx] = app_read_attributes(app_idx);
        }
    }

    int ret = modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX, &state->autoboot_item_idx);
#ifndef NDEBUG
    printf("MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX %u ret %i\n", state->autoboot_item_idx, ret);
#endif
    if (ret && state->autoboot_item_idx < state->boot_list.items_count)
        state->boot_list.hilite_idx = state->autoboot_item_idx;
}
