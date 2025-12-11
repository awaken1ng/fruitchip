#include <pico/types.h>
#include <pico/binary_info/defs.h>
#include <pico/binary_info/structure.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

inline static uint32_t *binary_info_find_marker(void *program)
{
    static const uintptr_t BOOT2_SIZE = 256;

    for (uintptr_t offset = 0; offset < BOOT2_SIZE; offset += 4)
    {
        uint32_t *marker_start = (void *)(program + BOOT2_SIZE + offset);
        if (*marker_start != BINARY_INFO_MARKER_START)
            continue;

        uint32_t *marker_end = (void *)(program + BOOT2_SIZE + offset + 16);
        if (*marker_end != BINARY_INFO_MARKER_END)
            continue;

        return marker_start;
    }

    return NULL;
}

inline static void *binary_info_map_address(uint32_t *mapping_table, void *addr)
{
    int i = 0;

    while (true)
    {
        uint32_t source_addr_start = *(mapping_table + i + 0);
        if (source_addr_start == 0)
            break;

        uint32_t dest_addr_start = *(mapping_table + i + 1);
        uint32_t dest_addr_end   = *(mapping_table + i + 2);

        bool in_range = ((uintptr_t)addr >= dest_addr_start) && ((uintptr_t)addr <= dest_addr_end);
        if (!in_range)
            goto next;

        return (void *)(source_addr_start + (uintptr_t)addr - dest_addr_start);

next:
        i += 3;
    }

    return addr;
}

inline static const char *binary_info_version(void *program)
{
    uint32_t *marker_start = binary_info_find_marker(program);
    if (!marker_start)
        return NULL;

    uint32_t *entries_start = (uint32_t *) *(marker_start + 1);
    uint32_t *entries_end   = (uint32_t *) *(marker_start + 2);
    uint32_t *mapping_table = (uint32_t *) *(marker_start + 3);

    uint32_t entries_size = (void *)entries_end - (void *)entries_start;
    uint32_t entries_count = entries_size / sizeof(uint32_t);
    for (uint32_t i = 0; i < entries_count; i++)
    {
        uint32_t *entry = (void *) *(entries_start + i);

        binary_info_core_t *core = (void *)entry;
        if (core->tag != BINARY_INFO_TAG_RASPBERRY_PI)
            continue;

        switch (core->type)
        {
            case BINARY_INFO_TYPE_ID_AND_STRING:
            {
                binary_info_id_and_string_t *idstr = (void *)entry;
                switch (idstr->id)
                {
                    case BINARY_INFO_ID_RP_PROGRAM_VERSION_STRING:
                        return binary_info_map_address(mapping_table, (void *)idstr->value);
                    default:
                        continue;
                }
            }
            default:
                continue;
        }
    }

    return NULL;
}
