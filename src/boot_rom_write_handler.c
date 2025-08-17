#include "boot_rom_write_handler.h"
#include "dma.h"

// ELF loader that launches stage 3 ELF
const uint8_t __not_in_flash("ee_stage2") LOADER_EE_STAGE_2[] = {
    #embed "../loader/ee-stage2/bin/ee-stage2.bin"
};

const uint8_t __not_in_flash("ee_stage3") LOADER_EE_STAGE_3[] = {
    #embed "../ps2bbl/bin/COMPRESSED_PS2BBL.ELF"
};

void handle_write_idle(uint8_t w);

void (*write_handler)(uint8_t) = &handle_write_idle;

void __time_critical_func(handle_write_payload_ee_stage3)(uint8_t w)
{
    if (w == 0xCC) dma_data_out_start_transfer(LOADER_EE_STAGE_3, sizeof(LOADER_EE_STAGE_3));
    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_payload_ee_stage2)(uint8_t w)
{
    if (w == 0xCB) dma_data_out_start_transfer(LOADER_EE_STAGE_2, sizeof(LOADER_EE_STAGE_2));
    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    switch (w)
    {
        case 0xC0: write_handler = &handle_write_payload_ee_stage2; break;
        case 0xC1: write_handler = &handle_write_payload_ee_stage3; break;
    }
}
