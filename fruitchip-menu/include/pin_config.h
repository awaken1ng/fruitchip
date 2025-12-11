#include <tamtypes.h>

void pin_config_get_from_update(
    u8 *fw,
    u32 *pin_mask_data,
    u32 *pin_mask_ce,
    u32 *pin_mask_oe,
    u32 *pin_mask_rst
);

void pin_config_get_current(
    u32 *pin_mask_data,
    u32 *pin_mask_ce,
    u32 *pin_mask_oe,
    u32 *pin_mask_rst
);
