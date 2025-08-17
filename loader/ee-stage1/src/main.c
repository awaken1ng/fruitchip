#include <kernel.h>
#include <sio.h>

#include "modchip.h"
#include "fletcher16.h"

static u16 __attribute__((section(".size"))) EE_STAGE_1_SIZE;
static u16 __attribute__((section(".checksum"))) EE_STAGE_1_CHECKSUM;

void __attribute__((section(".entry"))) __start()
{
    u16 checksum = fletcher16((void *)EE_STAGE_1_ADDR, EE_STAGE_1_SIZE);
    if (checksum == EE_STAGE_1_CHECKSUM)
        sio_puts("EE1: self-check ok");
    else
        sio_puts("EE1: self-check bad");

    u32 try = 0;
    while (true)
    {
        if (try > 32)
        {
            sio_puts("EE1: hit retry limit");
            asm volatile("break\n");
        }

        sio_puts("EE1: cmd");
        modchip_poke(0xC0);
        modchip_poke(0xCB);

        sio_puts("EE1: reading");
        for (u64 i = 0; i < EE_STAGE_2_SIZE; i += 4)
            *(volatile u32 *)(EE_STAGE_2_ADDR + i) = modchip_peek();

        u16 checksum = fletcher16((void *)EE_STAGE_2_ADDR, EE_STAGE_2_SIZE);
        if (checksum == 0x0000)
        {
            sio_puts("EE1: read fail");
            asm volatile("break\n");
        }

        if (checksum != EE_STAGE_2_CHECKSUM)
        {
            sio_puts("EE1: read bad");
            try += 1;
            continue;
        }

        sio_puts("EE1: read ok");
        break;
    }

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    sio_puts("EE1: jump");
    void ( *ee_stage2)(void) = (void (*)())EE_STAGE_2_ADDR;
    ee_stage2();

    __builtin_unreachable();
}
