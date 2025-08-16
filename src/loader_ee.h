#ifndef __LOADER_EE_H__
#define __LOADER_EE_H__

#include <stdint.h>

#include <pico/platform/sections.h>

// OSDSYS patch that launches stage 2
const uint8_t __not_in_flash("ee_stage1") LOADER_EE_STAGE1[] = {
    #embed "../loader/ee-stage1/bin/ee-stage1.bin"
};

// ELF loader that launches stage 3 ELF
const uint8_t __not_in_flash("ee_stage2") LOADER_EE_STAGE2[] = {
    #embed "../loader/ee-stage2/bin/ee-stage2.bin"
};

const uint8_t __not_in_flash("ee_stage3") LOADER_EE_STAGE3[] = {
    #embed "../ps2bbl/bin/COMPRESSED_PS2BBL.ELF"
};

static_assert(sizeof(LOADER_EE_STAGE1) <= 0x62C);

#endif /* __LOADER_EE_H__ */
