#include <pico/types.h>
#include <pico/binary_info/defs.h>
#include <pico/binary_info/structure.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

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

    return NULL;
}

inline static const char *binary_info_version(void *program)
{
    uint32_t *entries_start = NULL;
    uint32_t *entries_end = NULL;
    uint32_t *mapping_table = NULL;

    // look for BINARY_INFO_MARKER_START in 256 bytes after boot2
    for (int offset = 0; offset < 0x100; offset += 4)
    {
        uint32_t *marker_start = (void *)(program + 0x100 + offset);
        if (*marker_start != BINARY_INFO_MARKER_START)
            continue;

        uint32_t *marker_end = (void *)(program + 0x100 + offset + 16);
        if (*marker_end != BINARY_INFO_MARKER_END)
            return NULL;

        entries_start = (void *) *(uint32_t *)(program + 0x100 + offset + 4);
        entries_end   = (void *) *(uint32_t *)(program + 0x100 + offset + 8);
        mapping_table = (void *) *(uint32_t *)(program + 0x100 + offset + 12);

        break;
    }

    if (!entries_start || !entries_end || !mapping_table)
    {
        return NULL;
    }

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
