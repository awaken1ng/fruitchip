#include <kernel.h>
#include <libgs.h>

#include "modchip/io.h"
#include "modchip/cmd.h"
#include "crc32.h"

#ifndef NDEBUG
#include <sio.h>
#include "sio_ext.h"

#define print_debug(str) sio_puts(str)
#else
#define print_debug(str)
#endif

inline static void panic(const char *msg)
{
    print_debug(msg);
    GS_SET_BGCOLOR(0x70, 0x00, 0x00);
    asm volatile("break\n");
}

inline static void modchip_cmd_or_panic(u32 cmd)
{
    u32 ret;

    for (u8 attemp = 0; attemp < MODCHIP_CMD_RETRIES; attemp++)
    {
        modchip_poke_u32(cmd);
        ret = modchip_peek_u32();
        if (ret == MODCHIP_CMD_RESULT_OK) break;
    }

    if (ret == BOOT_ROM_ADDR_VALUE) panic("EE1: cmd no response");
    if (ret != MODCHIP_CMD_RESULT_OK) panic("EE1: cmd bad crc");
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
#ifndef NDEBUG
    sio_putsn("EE1: size 0x");
    sio_putxn(ee_stage1_size);
    sio_putsn(" crc 0x");
    sio_putxn(ee_stage1_crc);
    sio_putsn(" 0x");
    sio_putxn(crc);
    if (crc == ee_stage1_crc) sio_puts(" ok");
#endif
    if (crc != ee_stage1_crc) panic(" bad");

    modchip_cmd_or_panic(MODCHIP_CMD_GET_EE_STAGE2_SIZE);
    u32 ee_stage2_size = modchip_peek_u32();

    modchip_cmd_or_panic(MODCHIP_CMD_GET_EE_STAGE2);
    modchip_peek_n((void *)EE_STAGE_2_ADDR, ee_stage2_size);

    u32 ee_stage2_crc = modchip_peek_u32();
    crc = crc32((void *)EE_STAGE_2_ADDR, ee_stage2_size);
#ifndef NDEBUG
    sio_putsn("EE1: read 0x");
    sio_putxn(crc);
    if (crc == ee_stage2_crc) sio_puts(" ok");
#endif
    if (crc != ee_stage2_crc) panic(" bad");

    FlushCache(0); // flush data cache
    FlushCache(2); // invalidate instruction cache

    print_debug("EE1: jump");
    void ( *ee_stage2)(void) = (void (*)())EE_STAGE_2_ADDR;
    ee_stage2();

    __builtin_unreachable();
}
