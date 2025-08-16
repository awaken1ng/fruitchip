#ifndef __LOADELF_H__
#define __LOADELF_H__

#include <string.h>

#include <kernel.h>
#include <sio.h>

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

inline static void ExecPS2FromMemory(u8 *elf, int argc, char *argv[])
{
    elf_header_t *eh;
    elf_pheader_t *eph;
    void *pdata;
    int i;

    eh = (elf_header_t *)elf;
    if (_lw((u32)&eh->ident) != ELF_MAGIC)
    {
        sio_puts("not an elf");
        asm volatile("break\n");
    }

    eph = (elf_pheader_t *)(elf + eh->phoff);

    // Scan through the ELF's program headers and copy them into RAM,
    // then zero out any non-loaded regions.
    for (i = 0; i < eh->phnum; i++)
    {
        if (eph[i].type != ELF_PT_LOAD)
            continue;

        pdata = (void *)(elf + eph[i].offset);
        memcpy(eph[i].vaddr, pdata, eph[i].filesz);

        if (eph[i].memsz > eph[i].filesz)
            memset(eph[i].vaddr + eph[i].filesz, 0,
                   eph[i].memsz - eph[i].filesz);
    }

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    ExecPS2((void *)eh->entry, NULL, argc, argv);
}

#endif // __LOADELF_H__
