#include <string.h>

#include <kernel.h>
#include <sio.h>

#include "modchip.h"
#include "fletcher16.h"
#include "load_elf.h"

#define USER_MEM_START_ADDR 0x100000
#define USER_MEM_END_ADDR 0x2000000

void __attribute__((section(".entry"))) __start()
{
    sio_puts("EE2: clear");
    memset((void *)USER_MEM_START_ADDR, 0, USER_MEM_END_ADDR - USER_MEM_START_ADDR);

    sio_puts("EE2: cmd");
    modchip_poke(0xC1);
    modchip_poke(0xCC);

    sio_puts("EE2: read");
    for (u64 i = 0; i < EE_STAGE_3_SIZE; i += 4)
        *(volatile u32 *)(EE_STAGE_3_ADDR + i) = modchip_peek();

    u16 checksum = fletcher16((void *)EE_STAGE_3_ADDR, EE_STAGE_3_SIZE);
    if (checksum == EE_STAGE_3_CHECKSUM)
        sio_puts("EE2: checksum ok");
    else
        sio_puts("EE2: checksum bad");

    sio_puts("EE2: init");
    _InitSys();

    sio_puts("EE2: exec");
    char *argv[2];
    argv[0] = "payload";
    argv[1] = "payload";
    ExecPS2FromMemory((void *)EE_STAGE_3_ADDR, 2, argv);
}