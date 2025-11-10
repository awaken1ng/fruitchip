#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <tamtypes.h>

#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "wstring.h"
#include "constants.h"
#include "components/list.h"
#include "state.h"
#include "boot_list.h"

void apps_list_populate(struct state *state)
{
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
        for (u8 idx = 0; idx < apps_count; idx++)
        {
            u8 name_len = *ptr++;
            list_item.left_text = wstring_new_copied_str((char *)ptr, name_len);
            ptr += name_len;
            list_push_item(&state->boot_list, &list_item);
        }
    }

    int ret = modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX, &state->autoboot_item_idx);
    if (!ret || state->autoboot_item_idx >= state->boot_list.items_count)
        state->boot_list.hilite_idx = BOOT_ITEM_OSDSYS;
    printf("MODCHIP_SETTINGS_MENU_AUTOBOOT_ITEM_IDX %u\n", state->boot_list.hilite_idx);
}