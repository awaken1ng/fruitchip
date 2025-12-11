#include "modchip/flash.h"

#include <components/font.h>
#include <components/progress_bar.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <scene/message.h>
#include <constants.h>
#include <utils.h>

#define FLASH_ERASE_BYTE 0xFF

static u8 *update;
static u32 current_offset;
static u32 size;

static void pop_scene_and_show_message(struct state *state, wchar_t *text)
{
    superscene_pop_scene();
    scene_switch_to_message(state, text);
}

static bool write_update_sector(
    enum modchip_partition partition_idx,
    u8 *update,
    u32 offset,
    u32 sector_size
)
{
    bool whole_sector = size >= (offset + sector_size);

    u8 *sector = malloc(sector_size);
    if (!whole_sector)
    {
        memset(sector, FLASH_ERASE_BYTE, sector_size);
    }

    memcpy(sector, update + offset, sector_size);

    u32 sector_idx = offset /sector_size;
    int ret = modchip_write_flash_sector_with_retry(partition_idx, sector_idx, sector, sector_size, MODCHIP_CMD_RETRIES);

    free(sector);

    return ret;
}

static void scene_tick_handler_update_writing(struct state *state)
{
    enum modchip_partition partition_idx;

    switch (state->update_type)
    {
        case UPDATE_TYPE_FW:
            partition_idx = MODCHIP_PARTITION_IDX_FW_UPDATE;
            break;
        case UPDATE_TYPE_APPS:
            partition_idx = MODCHIP_PARTITION_IDX_APPS;
            break;
    }

    if (!modchip_set_write_lock_with_retry(false, MODCHIP_CMD_RETRIES))
    {
        pop_scene_and_show_message(state, L"Failed to unlock the write lock");
        goto exit;
    }

    if (!modchip_erase_flash_sector_with_retry(partition_idx, 0, 1, MODCHIP_CMD_RETRIES))
    {
        pop_scene_and_show_message(state, L"Failed to erase first sector");
        goto exit;
    }

    u32 sector_size;
    if (!modchip_get_flash_sector_size_with_retry(&sector_size, MODCHIP_CMD_RETRIES))
    {
        pop_scene_and_show_message(state, L"Failed to get flash sector size");
        goto exit;
    }

    // start from second sector
    for (current_offset = sector_size; current_offset < size; current_offset += sector_size)
    {
        if (!write_update_sector(partition_idx, update, current_offset, sector_size))
        {
            pop_scene_and_show_message(state, L"Failed to write flash sector");
            goto exit;
        }

        superscene_paint_after_vsync(state);
    }

    // first sector
    if (!write_update_sector(partition_idx, update, 0, sector_size))
    {
        pop_scene_and_show_message(state, L"Failed to write flash sector");
        goto exit;
    }

    superscene_pop_scene();
    scene_switch_to_update_rebooting();

exit:
    if (!modchip_set_write_lock_with_retry(true, MODCHIP_CMD_RETRIES))
    {
        print_debug("Failed to lock the write lock\n");
    }
}

static void scene_paint_handler_update_writing(struct state *state)
{
    float progress = (float)current_offset / (float)size;
    progress_bar_paint_center_with_message(state->gs, progress,  L"Writing update file");

    superscene_clear_button_guide(state);
}

void scene_switch_to_update_writing(u8 *_update, u32 _update_size)
{
    scene_t scene;
    scene_init(&scene);
    scene.tick_handler = scene_tick_handler_update_writing;
    scene.paint_handler = scene_paint_handler_update_writing;
    superscene_push_scene(scene);

    update = _update;
    current_offset = 0;
    size = _update_size;
}
