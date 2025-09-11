#include "pico/platform/sections.h"

#include <boot_rom/handler.h>
#include "idle.h"
#include "osdsys.h"
#include <boot_rom/write/disable_next_osdsys_hook.h>
#include <boot_rom/loader.h>
#include <boot_rom/data_out.h>

// assuming the first `OSDSYS` on the bus is going to be romdir entry,
// remember file size, then watch for ELFs and compare p_memsz to find OSDSYS,
// then look for ExecPS2 syscall to inject stage 1 payload into

static uint32_t counter = 0;
static uint32_t osdsys_size = 0;
static uint32_t bytes_read = 0;

void handle_read_find_hook_osdsys(uint8_t);
void handle_read_find_osdsys_elf(uint8_t);

void __time_critical_func(handle_read_hook_osdsys)(uint8_t r)
{
    counter += 1;

    //     1  2  3   4  5  6  7   8  9 10 11  12 13 14 15
    // --------------------------------------------------
    // 06 00 03 24  0C 00 00 00  08 00 E0 03  00 00 00 00
    // 07 00 03 24  0C 00 00 00  08 00 E0 03  00 00 00 00

    switch (counter)
    {
        case 1: if (r != 0x00) { goto exit; } break;
        case 2: if (r != 0x03) { goto exit; } break;
        case 3: if (r != 0x24) { goto exit; } break;

        case 4: if (r != 0x0C) { goto exit; } break;
        case 5:
        case 6:
        case 7: if (r != 0x00) { goto exit; } break;

        case 8: if (r != 0x08) { goto exit; } break;
        case 9: if (r != 0x00) { goto exit; } break;
        case 10: if (r != 0xE0) { goto exit; } break;
        case 11: if (r != 0x03) { goto exit; } break;

        case 12:
        case 13:
        case 14: if (r != 0x00) { goto exit; } break;
        case 15: if (r != 0x00) { goto exit; }
            if (!disable_next_osdsys_hook)
                boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_1, LOADER_EE_STAGE_1_SIZE, true);

            disable_next_osdsys_hook = false;
            read_handler = handle_read_find_osdsys_elf;
            counter = 0;
            break;

exit:
        default:
            read_handler = handle_read_find_hook_osdsys;
            counter = 0;
    }
}

void __time_critical_func(handle_read_find_hook_osdsys)(uint8_t r)
{
    bytes_read += 1;

    if (bytes_read >= osdsys_size / 2)
    {
        // failed to find the syscall table
        read_handler = handle_read_find_osdsys_elf;
    }
    else if (r == 0x06)
    {
        read_handler = handle_read_hook_osdsys;
    }
}

void __time_critical_func(handle_read_find_osdsys_elf)(uint8_t r)
{
    static uint32_t p_memsz = 0;

    counter += 1;

    switch (counter)
    {
        case 1: if (r != 0x7F) { goto exit; } break;
        case 2: if (r != 0x45) { goto exit; } break;
        case 3: if (r != 0x4C) { goto exit; } break;
        case 4: if (r != 0x46) { goto exit; } break;
        case 73: p_memsz = r; break;
        case 74: p_memsz |= r << 8; break;
        case 75: p_memsz |= r << 16; break;
        case 76: p_memsz |= r << 24;
            if (p_memsz == osdsys_size)
            {
                read_handler = handle_read_find_hook_osdsys;
                bytes_read = 0;
            }
exit:
        [[fallthrough]];
        default:
            if (counter >= 5 && counter <= 72) {} // skip to p_memsize
            else counter = 0;
    }
}

void __time_critical_func(handle_read_find_osdsys_size)(uint8_t r)
{
    counter += 1;

    //  0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15
    // ---------------------------------------------------
    // 4F 53 44 53  59 53 00 00  00 00 xx xx  yy yy yy yy | OSDSYS..........
    //                                 |      |- file size
    //                                 |- extinfo size

    switch (counter)
    {
        case 1: if (r != 0x53) { goto exit; } break;
        case 2: if (r != 0x44) { goto exit; } break;
        case 3: if (r != 0x53) { goto exit; } break;
        case 4: if (r != 0x59) { goto exit; } break;
        case 5: if (r != 0x53) { goto exit; } break;
        case 6:
        case 7:
        case 8:
        case 9: if (r != 0x00) { goto exit; } break;

        case 10: break; // extinfo size
        case 11: break;

        case 12: osdsys_size = r; break;
        case 13: osdsys_size |= r << 8; break;
        case 14: osdsys_size |= r << 16; break;
        case 15: osdsys_size |= r << 24;
            // file size in ELF header, will not match for protokernel OSDSYS
            osdsys_size -= 556;
            read_handler = handle_read_find_osdsys_elf;
            counter = 0;
            break;
exit:
        default:
            read_handler = handle_read_idle;
            counter = 0;
    }
}

