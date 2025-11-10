#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "uf2.h"
#include "modchip/flash.h"
#include "state.h"
#include "utils.h"
#include "scene/superscene.h"
#include "scene/message.h"
#include "update.h"
#include "boot_list.h"

#define FLASH_ERASE_BYTE 0xFF

static int open_fw_update_file()
{
    return open("mass:/" FW_UPDATE_PATH, O_RDONLY);
}

static int open_apps_update_file()
{
    return open("mass:/" APPS_UPDATE_PATH, O_RDONLY);
}

bool update_is_fw_update_file_present()
{
    int fd = open_fw_update_file();

    if (fd >= 0)
    {
        close(fd);
        return true;
    }

    return false;
}

bool update_is_apps_update_file_present()
{
    int fd = open_apps_update_file();

    if (fd >= 0)
    {
        close(fd);
        return true;
    }

    return false;
}

static bool check_fw_update_size(struct state *state, u32 update_size)
{
    u32 fw_partition_size, update_partition_size;
    bool failed = !modchip_get_partition_size(MODCHIP_PARTITION_IDX_FW, &fw_partition_size);
    failed |= !modchip_get_partition_size(MODCHIP_PARTITION_IDX_FW_UPDATE, &update_partition_size);

    if (failed)
    {
        scene_switch_to_message(state, L"Failed to get partition size");
        return false;
    }

    if (update_size > fw_partition_size || update_size > update_partition_size)
    {
        scene_switch_to_message(state, L"Not enough space to write the update");
        return false;
    }

    return true;
}

static bool check_apps_update_size(struct state *state, u32 update_size)
{
    u32 apps_partition_size;
    bool failed = !modchip_get_partition_size(MODCHIP_PARTITION_IDX_APPS, &apps_partition_size);

    if (failed)
    {
        scene_switch_to_message(state, L"Failed to get partition size");
        return false;
    }

    if (update_size > apps_partition_size)
    {
        scene_switch_to_message(state, L"Not enough space to write the update");
        return false;
    }

    return true;
}

static bool write_uf2_sector(
    struct state *state,
    struct UF2_Block *uf2,
    u32 sector_size,
    u32 block_size,
    u32 blocks_per_sector,
    enum modchip_partition partition_idx,
    u32 sector_idx,
    u32 block_count
)
{
    u8 *sector = malloc(sector_size);
    if (block_count != blocks_per_sector)
        memset(sector, FLASH_ERASE_BYTE, sector_size);

    for (u32 block_idx = 0; block_idx < block_count; block_idx++)
    {
        memcpy(
            sector + (block_idx * block_size),
            uf2[(sector_idx * blocks_per_sector) + block_idx].data,
            block_size
        );
    }

    int ret = modchip_write_flash_sector(partition_idx, sector_idx, sector, sector_size);

    free(sector);

    state->update_block_current += block_count;
    superscene_paint_after_vsync(state);
    printf("Wrote sector: sector_idx=%u sector_size=%u result=%x\n", sector_idx, sector_size, ret);

    return ret;
}

static void reboot_modchip_and_wait(struct state *state, enum modchip_reboot_mode mode)
{
    u64 reboot_start = clock_us();
    u64 reboot_deadline = reboot_start + (1000000 * 60); // one minute
    if (modchip_reboot(mode))
    {
        // wait for reboot
        while (true)
        {
            if (modchip_ping()) break;

            sleep(1);
            if (clock_us() > reboot_deadline)
            {
                scene_switch_to_message(state, L"Failed to reboot the modchip, timed out");
                break;
            }
        }
    }
    else
    {
        scene_switch_to_message(state, L"Failed to reboot the modchip");
    }
}

void update_start_fw_update(struct state *state)
{
    struct UF2_Block *uf2 = NULL;

    u32 sector_size;
    bool failed = !modchip_get_flash_sector_size(&sector_size);
    if (failed)
    {
        scene_switch_to_message(state, L"Failed to get flash sector size");
        return;
    }
    printf("Sector size: %i\n", sector_size);

    int fd = open_fw_update_file();
    if (fd < 0)
        return;

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (size % sizeof(struct UF2_Block) != 0)
    {
        scene_switch_to_message(state, L".uf2 is corrupted, invalid file size");
        goto exit;
    }

    uf2 = malloc(size);
    int bytes_read = read(fd, (void *)uf2, size);
    if (bytes_read != size)
    {
        scene_switch_to_message(state, L"Failed to read .uf2 file");
        goto exit;
    }

    state->update_block_current = 0;
    state->update_blocks_total = size / sizeof(struct UF2_Block);
    u32 update_size = 0;
    u32 block_size = uf2[0].payloadSize;

    // validate the blocks
    for (u32 i = 0; i < state->update_blocks_total; i++)
    {
        bool is_magic_valid =
            uf2[i].magicStart0 == UF2_MAGIC_START0 &&
            uf2[i].magicStart1 == UF2_MAGIC_START1 &&
            uf2[i].magicEnd == UF2_MAGIC_END;
        if (!is_magic_valid)
        {
            scene_switch_to_message(state, L".uf2 is corrupted, invalid magic");
            goto exit;
        }

        if (uf2[i].flags & UF2_FLAG_FAMILY_ID_PRESENT)
        {
            bool is_family_id_valid = uf2[i].fileSize == UF2_FAMILY_ID_RP2040;
            if (!is_family_id_valid)
            {
                scene_switch_to_message(state, L".uf2 is corrupted, invalid family id");
                goto exit;
            }
        }

        if (block_size != uf2[i].payloadSize)
        {
            scene_switch_to_message(state, L".uf2 block size is not consistent");
            goto exit;
        }

        update_size += uf2[i].payloadSize;
    }

    failed |= !check_fw_update_size(state, update_size);
    if (failed)
    {
        goto exit;
    }

    // write the blocks
    u64 update_start_us = clock_us();
    state->update_in_progress = true;

    if (!modchip_set_write_lock(false))
    {
        scene_switch_to_message(state, L"Failed to unlock the write lock");
        goto exit;
    }

    if (!modchip_erase_flash_sector(MODCHIP_PARTITION_IDX_FW_UPDATE, 0, 1))
    {
        scene_switch_to_message(state, L"Failed to erase first FW update sector");
        goto exit;
    }

    u32 blocks_per_sector = sector_size / block_size;
    u32 whole_sectors = state->update_blocks_total / blocks_per_sector;
    u32 remainder_blocks = state->update_blocks_total % blocks_per_sector;
    printf("Whole sectors: %i\n", whole_sectors);
    printf("Remainder blocks: %i\n", remainder_blocks);
    printf("Blocks total: %i\n", state->update_blocks_total);

    // start from second sector
    for (u32 sector_idx = 1; sector_idx < whole_sectors; sector_idx++)
    {
        failed |= !write_uf2_sector(
            state, uf2, sector_size, block_size, blocks_per_sector,
            MODCHIP_PARTITION_IDX_FW_UPDATE, sector_idx, blocks_per_sector
        );

        if (failed) goto exit;
    }

    // last unaligned sector, padded with 0xFF
    if (remainder_blocks)
    {
        failed |= !write_uf2_sector(
            state, uf2, sector_size, block_size, blocks_per_sector,
            MODCHIP_PARTITION_IDX_FW_UPDATE, whole_sectors, remainder_blocks
        );

        if (failed) goto exit;
    }

    // first sector
    failed |= !write_uf2_sector(
        state, uf2, sector_size, block_size, blocks_per_sector,
        MODCHIP_PARTITION_IDX_FW_UPDATE, 0, blocks_per_sector
    );

    if (failed) goto exit;

    u64 update_end_us = clock_us();
    u64 update_took_us = update_end_us - update_start_us;
    printf("FW update took %llu us\n", update_took_us);

    if (!modchip_set_write_lock(true))
    {
        printf("Failed to lock the write lock\n");
    }

    printf("Update complete, rebooting modchip\n");
    reboot_modchip_and_wait(state, MODCHIP_REBOOT_MODE_UPDATE);

    state->repaint = true;
    state->update_complete = true;

exit:
    if (failed)
    {
        scene_switch_to_message(state, L"Failed to write flash sector");
    }

    state->update_in_progress = false;
    free(uf2);
    close(fd);
}

void update_start_apps_update(struct state *state)
{
    struct UF2_Block *uf2 = NULL;

    u32 sector_size;
    bool failed = !modchip_get_flash_sector_size(&sector_size);
    if (failed)
    {
        scene_switch_to_message(state, L"Failed to get flash sector size");
        return;
    }
    printf("Sector size: %i\n", sector_size);

    int fd = open_apps_update_file();
    if (fd < 0)
        return;

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (size % sizeof(struct UF2_Block) != 0)
    {
        scene_switch_to_message(state, L".uf2 is corrupted, invalid file size");
        goto exit;
    }

    uf2 = malloc(size);
    int bytes_read = read(fd, (void *)uf2, size);
    if (bytes_read != size)
    {
        scene_switch_to_message(state, L"Failed to read .uf2 file");
        goto exit;
    }

    state->update_block_current = 0;
    state->update_blocks_total = size / sizeof(struct UF2_Block);
    u32 update_size = 0;
    u32 block_size = uf2[0].payloadSize;

    // validate the blocks
    for (u32 i = 0; i < state->update_blocks_total; i++)
    {
        bool is_magic_valid =
            uf2[i].magicStart0 == UF2_MAGIC_START0 &&
            uf2[i].magicStart1 == UF2_MAGIC_START1 &&
            uf2[i].magicEnd == UF2_MAGIC_END;
        if (!is_magic_valid)
        {
            scene_switch_to_message(state, L".uf2 is corrupted, invalid magic");
            goto exit;
        }

        if (uf2[i].flags & UF2_FLAG_FAMILY_ID_PRESENT)
        {
            bool is_family_id_valid = uf2[i].fileSize == UF2_FAMILY_ID_RP2040;
            if (!is_family_id_valid)
            {
                scene_switch_to_message(state, L".uf2 is corrupted, invalid family id");
                goto exit;
            }
        }

        if (block_size != uf2[i].payloadSize)
        {
            scene_switch_to_message(state, L".uf2 block size is not consistent");
            goto exit;
        }

        update_size += uf2[i].payloadSize;
    }

    failed |= !check_apps_update_size(state, update_size);
    if (failed)
    {
        goto exit;
    }

    // write the blocks
    u64 update_start_us = clock_us();
    state->update_in_progress = true;

    if (!modchip_set_write_lock(false))
    {
        scene_switch_to_message(state, L"Failed to unlock the write lock");
        goto exit;
    }

    if (!modchip_erase_flash_sector(MODCHIP_PARTITION_IDX_APPS, 0, 1))
    {
        scene_switch_to_message(state, L"Failed to erase apps header");
        goto exit;
    }

    u32 blocks_per_sector = sector_size / block_size;
    u32 whole_sectors = state->update_blocks_total / blocks_per_sector;
    u32 remainder_blocks = state->update_blocks_total % blocks_per_sector;
    printf("Whole sectors: %i\n", whole_sectors);
    printf("Remainder blocks: %i\n", remainder_blocks);
    printf("Blocks total: %i\n", state->update_blocks_total);

    // start from second sector
    for (u32 sector_idx = 1; sector_idx < whole_sectors; sector_idx++)
    {
        failed |= !write_uf2_sector(
            state, uf2, sector_size, block_size, blocks_per_sector,
            MODCHIP_PARTITION_IDX_APPS, sector_idx, blocks_per_sector
        );

        if (failed) goto exit;
    }

    // last unaligned sector, padded with 0xFF
    if (remainder_blocks)
    {
        failed |= !write_uf2_sector(
            state, uf2, sector_size, block_size, blocks_per_sector,
            MODCHIP_PARTITION_IDX_APPS, whole_sectors, remainder_blocks
        );

        if (failed) goto exit;
    }

    // first sector
    failed |= !write_uf2_sector(
        state, uf2, sector_size, block_size, blocks_per_sector,
        MODCHIP_PARTITION_IDX_APPS, 0, blocks_per_sector
    );

    if (failed) goto exit;

    u64 update_end_us = clock_us();
    u64 update_took_us = update_end_us - update_start_us;
    printf("Apps update took %llu us\n", update_took_us);

    if (!modchip_set_write_lock(true))
    {
        printf("Failed to lock the write lock\n");
    }

    printf("Update complete, rebooting modchip\n");
    reboot_modchip_and_wait(state, MODCHIP_REBOOT_MODE_SIMPLE);

    state->repaint = true;
    state->update_complete = true;

    apps_list_populate(state);

exit:
    if (failed)
    {
        scene_switch_to_message(state, L"Failed to write flash sector");
    }

    state->update_in_progress = false;
    free(uf2);
    close(fd);
}
