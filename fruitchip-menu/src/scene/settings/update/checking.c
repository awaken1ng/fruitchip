#include <fcntl.h>
#include <unistd.h>

#include <modchip/flash.h>

#include <uf2.h>

#include <components/font.h>
#include <components/progress_bar.h>
#include <scene/settings/update.h>
#include <scene/superscene.h>
#include <scene/message.h>
#include <pin_config.h>
#include <constants.h>

static u32 offset;
static u32 size;

static void pop_scene_and_show_message(struct state *state, wchar_t *text)
{
    superscene_pop_scene();
    scene_switch_to_message(state, text);
}

static void scene_tick_handler_update_checking(struct state *state)
{
    state->repaint = true;
    scene_paint_handler_superscene(state);

    int fd = update_file_open(state->update_type);
    if (fd < 0)
    {
        pop_scene_and_show_message(state, L"Failed to open update file");
        return;
    }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if ((size % sizeof(struct UF2_Block)) != 0)
    {
        pop_scene_and_show_message(state, L".uf2 is corrupted, invalid file size");
        goto exit;
    }

    static const u32 block_size = 256;

    static_assert(block_size == 256, "");
    u8 *update = malloc(size / 2);

    u32 last_block_addr = 0;
    u32 update_size = 0;

    for (offset = 0; offset < size; offset += sizeof(struct UF2_Block))
    {
        struct UF2_Block block;

        int bytes_read = read(fd, &block, sizeof(block));
        if (bytes_read != sizeof(block))
        {
            pop_scene_and_show_message(state, L"Failed to read .uf2 block");
            goto exit;
        }

        bool is_magic_valid =
            block.magicStart0 == UF2_MAGIC_START0 &&
            block.magicStart1 == UF2_MAGIC_START1 &&
            block.magicEnd == UF2_MAGIC_END;
        if (!is_magic_valid)
        {
            pop_scene_and_show_message(state, L"Invalid .uf2 magic");
            goto exit;
        }

        if (block.flags & UF2_FLAG_FAMILY_ID_PRESENT)
        {
            bool is_family_id_valid = block.fileSize == UF2_FAMILY_ID_RP2040;
            if (!is_family_id_valid)
            {
                pop_scene_and_show_message(state, L"Invalid .uf2 family id");
                goto exit;
            }
        }

        if (block_size != block.payloadSize)
        {
            pop_scene_and_show_message(state, L"Invalid .uf2 block size");
            goto exit;
        }

        if (last_block_addr && (last_block_addr + block_size) != block.targetAddr)
        {
            pop_scene_and_show_message(state, L".uf2 blocks are not linear");
            goto exit;
        }

        memcpy(update + update_size, block.data, block.payloadSize);
        update_size += block.payloadSize;

        last_block_addr = block.targetAddr;

        superscene_paint_after_vsync(state);
    }

    u32 fw_partition_size;
    u32 update_partition_size;
    u32 apps_partition_size;

    bool failed = !modchip_get_partition_size_with_retry(MODCHIP_PARTITION_IDX_FW, &fw_partition_size, MODCHIP_CMD_RETRIES);
    failed |= !modchip_get_partition_size_with_retry(MODCHIP_PARTITION_IDX_FW_UPDATE, &update_partition_size, MODCHIP_CMD_RETRIES);
    failed = !modchip_get_partition_size_with_retry(MODCHIP_PARTITION_IDX_APPS, &apps_partition_size, MODCHIP_CMD_RETRIES);

    if (failed)
    {
        pop_scene_and_show_message(state, L"Failed to get partition size");
        goto exit;
    }

    if (
        (state->update_type == UPDATE_TYPE_FW && (update_size > fw_partition_size || update_size > update_partition_size)) ||
        (state->update_type == UPDATE_TYPE_APPS && (update_size > apps_partition_size))
    )
    {
        pop_scene_and_show_message(state, L"Not enough space to write the update");
        goto exit;
    }

    if (state->update_type == UPDATE_TYPE_FW)
    {
        u32 pin_mask_data_current = 0;
        u32 pin_mask_ce_current = 0;
        u32 pin_mask_oe_current = 0;
        u32 pin_mask_rst_current = 0;

        pin_config_get_current(
            &pin_mask_data_current,
            &pin_mask_ce_current,
            &pin_mask_oe_current,
            &pin_mask_rst_current
        );

        if (
            !pin_mask_data_current ||
            !pin_mask_ce_current ||
            !pin_mask_oe_current ||
            !pin_mask_rst_current
        )
        {
            pop_scene_and_show_message(state, L"Failed to get current pin configuration");
            goto exit;
        }

        u32 pin_mask_data_update = 0;
        u32 pin_mask_ce_update = 0;
        u32 pin_mask_oe_update = 0;
        u32 pin_mask_rst_update = 0;

        pin_config_get_from_update(
            update,
            &pin_mask_data_update,
            &pin_mask_ce_update,
            &pin_mask_oe_update,
            &pin_mask_rst_update
        );

        if (
            pin_mask_data_update &&
            pin_mask_ce_update &&
            pin_mask_oe_update &&
            pin_mask_rst_update
        )
        {
            u32 pin_mask_current = pin_mask_data_current | pin_mask_ce_current | pin_mask_oe_current | pin_mask_rst_current;
            u32 pin_mask_update = pin_mask_data_update | pin_mask_ce_update | pin_mask_oe_update | pin_mask_rst_update;

            if (pin_mask_current != pin_mask_update)
            {
                pop_scene_and_show_message(state, L"Pin configuration does not match");
                goto exit;
            }
        }
    }

    superscene_pop_scene();
    scene_switch_to_update_writing(update, update_size);
    state->repaint = true;

exit:
    close(fd);
}

static void scene_paint_handler_update_checking(struct state *state)
{
    float progress = (float)offset / (float)size;
    progress_bar_paint_center_with_message(state->gs, progress,  L"Checking update file");

    superscene_clear_button_guide(state);
}

void scene_switch_to_update_checking()
{
    scene_t scene;
    scene_init(&scene);
    scene.tick_handler = scene_tick_handler_update_checking;
    scene.paint_handler = scene_paint_handler_update_checking;
    superscene_push_scene(scene);

    offset = 0;
    size = 0;
}
