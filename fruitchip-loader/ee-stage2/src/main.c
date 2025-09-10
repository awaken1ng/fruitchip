#include <string.h>

#include <kernel.h>
#include <sio.h>
#include <libgs.h>

#include <modchip/io.h>
#include <modchip/apps.h>
#include "load_elf.h"
#include "crc32.h"

#define USER_MEM_START_ADDR 0x100000
#define USER_MEM_END_ADDR 0x2000000
#define USER_MEM_SIZE (USER_MEM_END_ADDR - USER_MEM_START_ADDR)

DISABLE_PATCHED_FUNCTIONS();

inline static void panic(const char *msg)
{
    sio_puts(msg);
    GS_SET_BGCOLOR(0x70, 0x00, 0x00);
    asm volatile("break\n");
}

void __attribute__((section(".entry"))) __start()
{
    sio_puts("EE2: clear");
    memset((void *)USER_MEM_START_ADDR, 0, USER_MEM_SIZE);

    u32 ee_stage2_size;
    if (!modchip_apps_read(MODCHIP_APPS_SIZE_OFFSET, MODCHIP_APPS_SIZE_SIZE, 1, &ee_stage2_size))
        panic("EE2: size read failed");

    if (!modchip_apps_read(MODCHIP_APPS_DATA_OFFSET, ee_stage2_size, 1, (void *)EE_STAGE_3_ADDR))
        panic("EE2: data read failed");

    u32 ee_stage2_crc;
    if (!modchip_apps_read(MODCHIP_APPS_CRC_OFFSET(ee_stage2_size), MODCHIP_APPS_CRC_SIZE, 1, &ee_stage2_crc))
        panic("EE2: crc read failed");

    u32 crc = crc32((void *)EE_STAGE_3_ADDR, ee_stage2_size);
    crc == ee_stage2_crc ? sio_puts("EE2: crc ok") : sio_puts("EE2: crc bad");

    sio_puts("EE2: init");
    _InitSys();

    sio_puts("EE2: exec");
    static char *argv[2];
    argv[0] = "payload";
    ExecPS2FromMemory((void *)EE_STAGE_3_ADDR, 1, argv);
}