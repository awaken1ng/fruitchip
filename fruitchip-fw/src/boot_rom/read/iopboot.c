#include <pico/platform/sections.h>

#include <boot_rom/read/idle.h>
#include <boot_rom/read/osdsys.h>
#include <boot_rom/write/patch_control.h>
#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>
#include <boot_rom/loader.h>

static uint32_t counter = 0;

void __time_critical_func(handle_read_find_iopboot3)(uint8_t r)
{
    counter += 1;

    switch (counter)
    {
        // clear a3
        case 20: if (r != 0x21) { goto exit; } break;
        case 21: if (r != 0x38) { goto exit; } break;
        case 22: if (r != 0x00) { goto exit; } break;
        case 23: if (r != 0x00) { goto exit; } break;

        // jr v0
        case 24: if (r != 0x08) { goto exit; } break;
        case 25: if (r != 0x00) { goto exit; } break;
        case 26: if (r != 0x40) { goto exit; } break;
        case 27: if (r != 0x00) { goto exit; } break;

        // _clear     s0
        case 28: if (r != 0x21) { goto exit; } break;
        case 29: if (r != 0x80) { goto exit; } break;
        case 30: if (r != 0x00) { goto exit; } break;
        case 31: if (r != 0x00) { goto exit; };
            if (!disable_next_iopboot_hook)
            {
                boot_rom_data_out_start_data_without_status_code(false);
            }

exit:
        default:
            read_handler = handle_read_idle;
            counter = 0;
    }
}

void __time_critical_func(handle_read_find_iopboot2)(uint8_t r)
{
    counter += 1;

    switch (counter)
    {
        // move a0,s0
        case 8: if (r != 0x21) { goto exit; } break;
        case 9: if (r != 0x20) { goto exit; } break;
        case 10: if (r != 0x00) { goto exit; } break;
        case 11: if (r != 0x02) { goto exit; } break;

        // clear a1
        case 12: if (r != 0x21) { goto exit; } break;
        case 13: if (r != 0x28) { goto exit; } break;
        case 14: if (r != 0x00) { goto exit; } break;
        case 15: if (r != 0x00) { goto exit; } break;

        // clear a2
        case 16: if (r != 0x21) { goto exit; } break;
        case 17: if (r != 0x30) { goto exit; } break;
        case 18: if (r != 0x00) { goto exit; } break;
        case 19: if (r != 0x00) { goto exit; }
            if (!disable_next_iopboot_hook)
            {
                boot_rom_data_out_set_data(LOADER_IOP_STAGE_1, LOADER_IOP_STAGE_1_SIZE);
                read_handler = handle_read_find_iopboot3;
            }

            break;

exit:
        default:
            read_handler = handle_read_idle;
            counter = 0;
    }

}

void __time_critical_func(handle_read_find_iopboot)(uint8_t r)
{
    // somewhere around RESET+0x240c, there's function call with arguments of (0xbfc00000, 0xbfc80000, "IOPBOOT"),
    // and if returned value is not zero, we jump to the address

    // 0000246c 00 00 00 00     _nop
    // 00002470 07 00 40 10     beq        v0,zero,0x2490
    // 00002474 00 00 00 00     _nop
    // 00002478 21 20 00 02     move       a0,s0
    // 0000247c 21 28 00 00     clear      a1
    // 00002480 21 30 00 00     clear      a2
    // 00002484 21 38 00 00     clear      a3
    // 00002488 08 00 40 00     jr         v0
    // 0000248c 21 80 00 00     _clear     s0

    // look for this pattern and inject IOP payload instead of IOPBOOT

    counter += 1;

    switch (counter)
    {
        // beq v0, zero, 0x2490
        // case 0: if (r != 0x07) { goto exit; } break;
        case 1: if (r != 0x00) { goto exit; } break;
        case 2: if (r != 0x40) { goto exit; } break;
        case 3: if (r != 0x10) { goto exit; } break;

        // _nop
        case 4:
        case 5:
        case 6: if (r != 0x00) { goto exit; } break;
        case 7: if (r != 0x00) { goto exit; };
            read_handler = handle_read_find_iopboot2;
            break;
exit:
        default:
            read_handler = handle_read_idle;
            counter = 0;
    }
}