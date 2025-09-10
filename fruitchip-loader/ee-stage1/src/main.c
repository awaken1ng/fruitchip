#include <kernel.h>
#include <sio.h>

#include <modchip/io.h>
#include <modchip/cmd.h>
#include "crc32.h"

inline static void modchip_cmd_or_panic(u32 cmd)
{
    if (!modchip_cmd(cmd))
    {
        sio_puts("EE1: cmd failed");
        asm volatile("break\n");
    }
}

void __attribute__((section(".entry"))) __ExecPS2(void* entry, void* gp, int argc, char** argv)
{
    (void)entry; // 0x200000
    (void)gp; // NULL
    (void)argc; // 1
    (void)argv; // rom0:OSDSYS

    modchip_cmd_or_panic(MODCHIP_CMD_GET_EE_STAGE1_SIZE);
    u32 ee_stage1_size = modchip_peek_u32();

    u32 ee_stage1_crc = *(uiptr *)(EE_STAGE_1_ADDR + ee_stage1_size);
    u32 crc = crc32((void *)EE_STAGE_1_ADDR, ee_stage1_size);
    ee_stage1_crc == crc ? sio_puts("EE1: self-check ok") : sio_puts("EE1: self-check bad");

    modchip_cmd_or_panic(MODCHIP_CMD_GET_EE_STAGE2_SIZE);
    u32 ee_stage2_size = modchip_peek_u32();

    // payload seems to be aligned to 4 bytes anyway, and even if its not,
    // extra zeroes at the end should still be fine as long as we're under size limit
    modchip_cmd_or_panic(MODCHIP_CMD_GET_EE_STAGE2);
    for (uiptr i = 0; i < ee_stage2_size; i += 4)
        *(volatile u32 *)(EE_STAGE_2_ADDR + i) = modchip_peek_u32();

    u32 ee_stage2_crc = modchip_peek_u32();
    crc = crc32((void *)EE_STAGE_2_ADDR, ee_stage2_size);
    ee_stage2_crc == crc ? sio_puts("EE1: read ok") : sio_puts("EE1: read bad");

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    sio_puts("EE1: jump");
    void ( *ee_stage2)(void) = (void (*)())EE_STAGE_2_ADDR;
    ee_stage2();

    __builtin_unreachable();
}
