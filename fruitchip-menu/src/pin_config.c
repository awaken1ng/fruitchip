#include <string.h>
#include <uf2.h>

#include "modchip/config.h"
#include "binary_info_parser.h"

inline static void *convert_ram_address(uintptr_t base_address, void *ram_addr, void *program)
{
    return program + (uintptr_t)ram_addr - base_address;
}

void pin_config_get_from_update(
    u8 *fw,
    u32 *pin_mask_data,
    u32 *pin_mask_ce,
    u32 *pin_mask_oe,
    u32 *pin_mask_rst
)
{
    uint32_t *marker_start = binary_info_find_marker(fw);
    if (!marker_start)
        return;

    uint32_t *entries_start = (uint32_t *) (uintptr_t) *(marker_start + 1);
    uint32_t *entries_end   = (uint32_t *) (uintptr_t) *(marker_start + 2);
    uint32_t *mapping_table = (uint32_t *) (uintptr_t) *(marker_start + 3);

    static const uintptr_t BASE_ADDRESS = 0x10000000 + FW_PARTITION_OFFSET;

    mapping_table = convert_ram_address(BASE_ADDRESS, mapping_table, fw);

    uint32_t entries_size = (uintptr_t)entries_end - (uintptr_t)entries_start;
    uint32_t entries_count = entries_size / sizeof(uint32_t);

    for (uint32_t i = 0; i < entries_count; i++)
    {
        uint32_t *entry_ptr = convert_ram_address(BASE_ADDRESS, entries_start + i, fw);
        uint32_t *entry = convert_ram_address(BASE_ADDRESS, (void *)(uintptr_t)*entry_ptr, fw);

        binary_info_core_t *core = (void *)entry;
        if (core->tag != BINARY_INFO_TAG_RASPBERRY_PI)
            continue;

        switch (core->type)
        {
            case BINARY_INFO_TYPE_PINS_WITH_NAME:
            {
                binary_info_pins_with_name_t *pin_info = (void *)entry;

                const char *label = binary_info_map_address(mapping_table, (void *)(uintptr_t)pin_info->label);
                label = convert_ram_address(BASE_ADDRESS, (void *)label, fw);

                if (pin_mask_data)
                {
                    if (!strcmp(label, "Boot ROM Q0")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q1")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q2")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q3")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q4")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q5")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q6")) *pin_mask_data |= pin_info->pin_mask;
                    if (!strcmp(label, "Boot ROM Q7")) *pin_mask_data |= pin_info->pin_mask;
                }

                if (pin_mask_ce)
                {
                    if (!strcmp(label, "Boot ROM CE")) *pin_mask_ce |= pin_info->pin_mask;
                }

                if (pin_mask_oe)
                {
                    if (!strcmp(label, "Boot ROM OE")) *pin_mask_oe |= pin_info->pin_mask;
                }

                if (pin_mask_rst)
                {
                    if (!strcmp(label, "RST")) *pin_mask_rst |= pin_info->pin_mask;
                }
            }
        }
    }

    return;
}

void pin_config_get_current(
    u32 *pin_mask_data,
    u32 *pin_mask_ce,
    u32 *pin_mask_oe,
    u32 *pin_mask_rst
)
{
    if (pin_mask_data)
    {
        *pin_mask_data = modchip_pin_config(MODCHIP_PIN_BOOT_ROM_DATA);
    }

    if (pin_mask_ce)
    {
        *pin_mask_ce = modchip_pin_config(MODCHIP_PIN_BOOT_ROM_CE);
    }

    if (pin_mask_oe)
    {
        *pin_mask_oe = modchip_pin_config(MODCHIP_PIN_BOOT_ROM_OE);
    }

    if (pin_mask_rst)
    {
        *pin_mask_rst = modchip_pin_config(MODCHIP_PIN_RESET);
    }
}
