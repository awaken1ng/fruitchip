#include "pico/platform/sections.h"

#include <boot_rom/handler.h>
#include "idle.h"
#include "osdsys.h"
#include <boot_rom/write/disable_next_osdsys_hook.h>
#include <boot_rom/loader.h>
#include <boot_rom/data_out.h>

static uint32_t counter = 0;
static uint32_t bytes_read = 0;

void handle_read_find_hook_osdsys(uint8_t);
void handle_read_find_osdsys_elf(uint8_t);

void __time_critical_func(handle_read_hook_osdsys)(uint8_t r)
{
    // 06 00 03 24  0C 00 00 00  08 00 E0 03  00 00 00[00]
    // 07 00 03 24  0C 00 00 00  08 00 E0 03  00 00 00 00

    if (r != 0x00)
    {
        read_handler = handle_read_find_hook_osdsys;
        return;
    }

    if (!disable_next_osdsys_hook)
        boot_rom_data_out_start_data_without_status_code(true);

    read_handler = handle_read_find_osdsys_elf;
    disable_next_osdsys_hook = false;
}

void __time_critical_func(handle_read_find_osdsys_syscall_table)(uint8_t r)
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
        case 11: if (r != 0x03) { goto exit; }
            // setup data out early
            // helps avoding delay with data out start on cold boot
            boot_rom_data_out_set_data(LOADER_EE_STAGE_1, LOADER_EE_STAGE_1_SIZE);
            break;

        case 12:
        case 13: if (r != 0x00) { goto exit; } break;
        case 14: if (r != 0x00) { goto exit; }
            // move injection handler into it's own function,
            // otherwise, if kept here, injection becomes unreliable
            // (switch case getting too big?)
            read_handler = handle_read_hook_osdsys;
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

    if (bytes_read >= 1024)
    {
        // failed to find the syscall table
        read_handler = handle_read_find_osdsys_elf;
    }
    else if (r == 0x06)
    {
        read_handler = handle_read_find_osdsys_syscall_table;
    }
}

void __time_critical_func(handle_read_find_osdsys_elf)(uint8_t r)
{
    counter += 1;

    switch (counter)
    {
        case 1: if (r != 0x7F) { goto exit; } break;
        case 2: if (r != 0x45) { goto exit; } break; // E
        case 3: if (r != 0x4C) { goto exit; } break; // L
        case 4: if (r != 0x46) { goto exit; } break; // F

        // e_entry
        case 25: if (r != 0x08) { goto exit; } break;
        case 26: if (r != 0x00) { goto exit; } break;
        case 27: if (r != 0x10) { goto exit; } break;
        case 28: if (r != 0x00) { goto exit; } break;

        // e_shnum
        case 49: if (r != 0x08) { goto exit; } break;
        case 50: if (r != 0x00) { goto exit; } break;

        // e_shstrndx
        case 51: if (r != 0x07) { goto exit; } break;
        case 52: if (r != 0x00) { goto exit; }
            read_handler = handle_read_find_hook_osdsys;
            bytes_read = 0;
            counter  = 0;
            break;

exit:
        default:
            if (counter >= 5 && counter <= 24) {} // skip to e_entry
            else if (counter >= 29 && counter <= 48) {} // skip to e_shnum
            else counter = 0;
    }
}

