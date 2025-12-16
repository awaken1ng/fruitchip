#pragma once

#include <string.h>

#include <kernel.h>
#include <sio.h>

#include "modchip/apps.h"
#include "sio_ext.h"

#define ELF_MAGIC 0x464c457f
#define ELF_PT_LOAD 1

typedef struct
{
    u8 ident[16]; // struct definition for ELF object header
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} elf_header_t;

typedef struct
{
    u32 type; // struct definition for ELF program section header
    u32 offset;
    void *vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
} elf_pheader_t;

inline static bool ExecPS2FromModchipStage3(int argc, char *argv[])
{
    elf_header_t eh;
    elf_pheader_t eph;
    int i;

    s32 ret = modchip_stage3_read_with_retry(
        0,
        sizeof(eh),
        &eh,
        true,
        MODCHIP_CMD_RETRIES
    );
    if (ret)
    {
        sio_puts("failed to read elf header");
        return false;
    }

    if (_lw((u32)&eh.ident) != ELF_MAGIC)
    {
        sio_puts("not an elf");
        return false;
    }

    // Scan through the ELF's program headers and copy them into RAM,
    // then zero out any non-loaded regions.
    for (i = 0; i < eh.phnum; i++)
    {
        if (modchip_stage3_read_with_retry(
                eh.phoff + (sizeof(eph) * i),
                sizeof(eph),
                &eph,
                true,
                MODCHIP_CMD_RETRIES
            ))
        {
            sio_puts("failed to read elf section header");
            return false;
        }

        if (eph.type != ELF_PT_LOAD)
            continue;

        if (modchip_stage3_read_with_retry(
                eph.offset,
                eph.filesz,
                eph.vaddr,
                true,
                MODCHIP_CMD_RETRIES
            ))
        {
            sio_puts("failed to read elf section");
            return false;
        }

        if (eph.memsz > eph.filesz)
            memset(eph.vaddr + eph.filesz, 0, eph.memsz - eph.filesz);
    }

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    ExecPS2((void *)eh.entry, NULL, argc, argv);

    return true;
}

inline static bool ExecPS2FromModchipApps(u8 app_idx, int argc, char *argv[])
{
    elf_header_t eh;
    elf_pheader_t eph;
    int i;

    s32 ret = modchip_apps_read_with_retry(
        MODCHIP_APPS_DATA_OFFSET,
        sizeof(eh),
        app_idx,
        &eh,
        true,
        MODCHIP_CMD_RETRIES
    );
    if (ret)
    {
        sio_puts("failed to read elf header");
        return false;
    }

    if (_lw((u32)&eh.ident) != ELF_MAGIC)
    {
        sio_puts("not an elf");
        return false;
    }

    // Scan through the ELF's program headers and copy them into RAM,
    // then zero out any non-loaded regions.
    for (i = 0; i < eh.phnum; i++)
    {
        if (modchip_apps_read_with_retry(
                MODCHIP_APPS_DATA_OFFSET + eh.phoff + (sizeof(eph) * i),
                sizeof(eph),
                app_idx,
                &eph,
                true,
                MODCHIP_CMD_RETRIES
            ))
        {
            sio_puts("failed to read elf section header");
            return false;
        }

        if (eph.type != ELF_PT_LOAD)
            continue;

        if (modchip_apps_read_with_retry(
                MODCHIP_APPS_DATA_OFFSET + eph.offset,
                eph.filesz,
                app_idx,
                eph.vaddr,
                true,
                MODCHIP_CMD_RETRIES
            ))
        {
            sio_puts("failed to read elf section");
            return false;
        }

        if (eph.memsz > eph.filesz)
            memset(eph.vaddr + eph.filesz, 0, eph.memsz - eph.filesz);
    }

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    ExecPS2((void *)eh.entry, NULL, argc, argv);

    return true;
}
