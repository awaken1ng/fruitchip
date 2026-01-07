#include <pico/platform/sections.h>

#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>
#include <boot_rom/loader.h>
#include <boot_rom/read/idle.h>
#include <boot_rom/read/osdsys.h>
#include <boot_rom/write/disable_next_osdsys_hook.h>
#include <led_color.h>

static uint32_t counter = 0;

void __time_critical_func(inject_osdhook)()
{
    // Injects stage 1 code if not explicitly disabled
    if (!disable_next_osdsys_hook)
        boot_rom_data_out_start_data_without_status_code(true);

    // Reset to the idle handler and set LED to indicate that the hook was successful
    read_handler = handle_read_idle;
    disable_next_osdsys_hook = false;
    counter = 0;
    colored_status_led_set_on_with_color(RGB_OK_OSDHOOK);
}

void __time_critical_func(handle_read_find_osdsys_syscall_table)(uint8_t r)
{
    // The hook payload (stage 1) must be injected at this part of the syscall table, at syscall 0x7 (ExecPS2):
    //  0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15
    // --------------------------------------------------
    // 06 00 03 24  0C 00 00 00  08 00 E0 03  00 00[00]00
    // 07 00 03 24  0C 00 00 00  08 00 E0 03  00 00 00 00
    counter += 1;
    switch (counter)
    {
        case 507: // 00, byte 14
            if (r != 0x00) goto reset;
            // Inject 1 byte earlier to avoid timing issues
            inject_osdhook();
            break;
        case 504: // 03, byte 11
            if (r != 0x03) goto reset;
            // Setup data out early to avoid delay with data out start on cold boot
            boot_rom_data_out_set_data(LOADER_EE_STAGE_1, LOADER_EE_STAGE_1_SIZE);
            break;
        case 508: // 00, byte 15
            // Missed the injection window
            goto reset;
    }
    return;

reset:
    read_handler = handle_read_idle;
    counter = 0;
}

void __time_critical_func(handle_read_find_osdsys_elf)(uint8_t r)
{
    // This function looks for the beginning of the OSDSYS ELF header:
    // 7F 45 4C 46 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ??
    // ?? ?? ?? ?? ?? ?? ?? ?? 08 00 10 00 ?? ?? ?? ??
    // ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ??
    // 08 00 07 00
    counter += 1;
    switch (counter)
    {
        // ELF magic (0x7F454C46), 0x7F is processed by the handle_read_idle function
        case 1:
            if (r != 0x45) goto reset;
            break; // E
        case 2:
            if (r != 0x4C) goto reset;
            break; // L
        case 3:
            if (r != 0x46) goto reset;
            break; // F

        // e_entry (0x100008)
        case 24:
            if (r != 0x08) goto reset;
            break;
        case 25:
            if (r != 0x00) goto reset;
            break;
        case 26:
            if (r != 0x10) goto reset;
            break;
        case 27:
            if (r != 0x00) goto reset;
            break;

        // e_shnum (0x0008)
        case 48:
            if (r != 0x08) goto reset;
            break;
        case 49:
            if (r != 0x00) goto reset;
            break;

        // e_shstrndx (0x0007)
        case 50:
            if (r != 0x07) goto reset;
            break;
        case 51:
            if (r != 0x00) goto reset;

            // Full match, switch to the next read handler
            counter = 0;
            read_handler = handle_read_find_osdsys_syscall_table;
            return;
    }

    if ((r == 0x7F) || (counter > 51))
        goto reset;

    return;

reset:
    counter = 0;
}