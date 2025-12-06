#pragma once

#include <pico/binary_info.h>

#include <git_version.h>

bi_decl(bi_program_name("fruitchip-fw"));
bi_decl(bi_program_version_string(GIT_REV));
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 0, "Boot ROM Q0"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 1, "Boot ROM Q1"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 2, "Boot ROM Q2"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 3, "Boot ROM Q3"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 4, "Boot ROM Q4"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 5, "Boot ROM Q5"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 6, "Boot ROM Q6"))
bi_decl(bi_1pin_with_name(BOOT_ROM_Q0_PIN + 7, "Boot ROM Q7"))
bi_decl(bi_1pin_with_name(BOOT_ROM_CE_PIN,     "Boot ROM CE"))
bi_decl(bi_1pin_with_name(BOOT_ROM_OE_PIN,     "Boot ROM OE"))
bi_decl(bi_1pin_with_name(RST_PIN,             "RST"))
