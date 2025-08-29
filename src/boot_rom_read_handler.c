#include "boot_rom_read_handler.h"
#include "boot_rom_data_out.h"

// OSDSYS patch that launches stage 2
const uint8_t __not_in_flash("ee_stage1") LOADER_EE_STAGE_1[] = {
    #embed "../loader/ee-stage1/bin/ee-stage1.bin"
};

static_assert(sizeof(LOADER_EE_STAGE_1) <= 0x62C);

// assuming the first `OSDSYS` on the bus is going to be romdir entry,
// remember file size, then watch for ELFs and compare p_memsz to find OSDSYS,
// then look for ExecPS2 syscall to inject stage 1 payload into

void handle_read_idle(uint8_t r);
void handle_read_osdsys_0_4Fh_seen(uint8_t r);
void handle_read_osdsys_1_53h_seen(uint8_t r);
void handle_read_osdsys_2_44h_seen(uint8_t r);
void handle_read_osdsys_3_53h_seen(uint8_t r);
void handle_read_osdsys_4_59h_seen(uint8_t r);
void handle_read_osdsys_5_53h_seen(uint8_t r);
void handle_read_osdsys_size_seen(uint8_t r);

void handle_read_elf_0_7Fh_seen(uint8_t r);
void handle_read_elf_1_45h_seen(uint8_t r);
void handle_read_elf_2_4Ch_seen(uint8_t r);
void handle_read_elf_3_46h_seen(uint8_t r);
void handle_read_elf_osdsys_found(uint8_t r);

void handle_read_syscall_0_07h_seen(uint8_t r);
void handle_read_syscall_1_00h_seen(uint8_t r);
void handle_read_syscall_2_03h_seen(uint8_t r);

void (*read_handler)(uint8_t) = &handle_read_idle;

static uint32_t osdsys_size = 0;

void __time_critical_func(handle_read_idle)(uint8_t r)
{
    switch (r)
    {
        case 0x4F: read_handler = &handle_read_osdsys_0_4Fh_seen; break;
    }
}

// region: OSDSYS size

void __time_critical_func(handle_read_osdsys_0_4Fh_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x53: read_handler = &handle_read_osdsys_1_53h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_1_53h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x44: read_handler = &handle_read_osdsys_2_44h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_2_44h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x53: read_handler = &handle_read_osdsys_3_53h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_3_53h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x59: read_handler = &handle_read_osdsys_4_59h_seen; break;
        default: read_handler = &handle_read_idle; break;
    }
}

void __time_critical_func(handle_read_osdsys_4_59h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x53:
            read_handler = &handle_read_osdsys_5_53h_seen;
            break;
        default:
            read_handler = &handle_read_idle;
            break;
    }
}

void __time_critical_func(handle_read_osdsys_5_53h_seen)(uint8_t r)
{
    static uint8_t buffer[4] = {};
    static uint32_t counter = 0;

    // 4F 53 44 53  59 53 00 00  00 00 xx xx  yy yy yy yy | OSDSYS..........
    //                                 |      |- file size
    //                                 |- extinfo size

    counter += 1;

    // 4 bytes, zeroes
    if (counter <= 4 && r != 0x00)
    {
        counter = 0;
        read_handler = &handle_read_osdsys_size_seen;
    }

    // 2 bytes, extinfo size
    if (counter <= 6)
        return;

    // 4 bytes, file size
    if (counter >= 7 && counter <= 10)
        buffer[counter - 7] = r;

    if (counter == 10)
    {
        // file size in ELF header, will not match for protokernel OSDSYS
        osdsys_size = *(uint32_t*)&buffer - 556;

        counter = 0;
        read_handler = &handle_read_osdsys_size_seen;
    }
}

// endregion: OSDSYS size

void __time_critical_func(handle_read_osdsys_size_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x7F: read_handler = &handle_read_elf_0_7Fh_seen; break;
    }
}

void __time_critical_func(handle_read_elf_0_7Fh_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x45: read_handler = &handle_read_elf_1_45h_seen; break;
        default: read_handler = &handle_read_osdsys_size_seen; break;
    }
}

void __time_critical_func(handle_read_elf_1_45h_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x4C: read_handler = &handle_read_elf_2_4Ch_seen; break;
        default: read_handler = &handle_read_osdsys_size_seen; break;
    }
}

void __time_critical_func(handle_read_elf_2_4Ch_seen)(uint8_t r)
{
    switch (r)
    {
        case 0x46: read_handler = &handle_read_elf_3_46h_seen; break;
        default: read_handler = &handle_read_osdsys_size_seen; break;
    }
}

void __time_critical_func(handle_read_elf_3_46h_seen)(uint8_t r)
{
    static uint8_t counter = 0;
    static bool match = false;

    counter += 1;

    // skip to p_memsz
    if (counter >= 69 && counter <= 72)
    {
        match = r == ((uint8_t*)&osdsys_size)[counter - 69];
        if (!match)
        {
            counter = 0;
            read_handler = &handle_read_osdsys_size_seen;
        }
    }

    if (counter >= 72)
    {
        if (match)
            read_handler = &handle_read_elf_osdsys_found;
        else
            read_handler = &handle_read_osdsys_size_seen;

        counter = 0;
        match = false;
    }
}

static uint32_t osdsys_bytes_read = 0;

void __time_critical_func(handle_read_elf_osdsys_found)(uint8_t r)
{
    osdsys_bytes_read += 1;

    // escape hatch in case we get stuck
    if (osdsys_bytes_read >= osdsys_size)
    {
        read_handler = &handle_read_osdsys_size_seen;
        osdsys_bytes_read = 0;
        return;
    }

    switch (r)
    {
        case 0x07:
            read_handler = &handle_read_syscall_0_07h_seen;
            break;
    }
}


void __time_critical_func(handle_read_syscall_0_07h_seen)(uint8_t r)
{
    osdsys_bytes_read += 1;

    switch (r)
    {
        case 0x00: read_handler = &handle_read_syscall_1_00h_seen; break;
        default: read_handler = &handle_read_elf_osdsys_found; break;
    }
}

void __time_critical_func(handle_read_syscall_1_00h_seen)(uint8_t r)
{
    osdsys_bytes_read += 1;

    switch (r)
    {
        case 0x03: read_handler = &handle_read_syscall_2_03h_seen; break;
        default: read_handler = &handle_read_elf_osdsys_found; break;
    }
}

void __time_critical_func(handle_read_syscall_2_03h_seen)(uint8_t r)
{
    switch (r)
    {
        [[fallthrough]];
        case 0x24:
            // 07 00 03[24] 0C 00 00 00  08 00 E0 03  00 00 00 00
            dma_data_out_start_transfer(LOADER_EE_STAGE_1, sizeof(LOADER_EE_STAGE_1));
            osdsys_bytes_read += 1;
        default:
            read_handler = &handle_read_osdsys_size_seen;
            osdsys_bytes_read = 0;
            break;
    }
}
