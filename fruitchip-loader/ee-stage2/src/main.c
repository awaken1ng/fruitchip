#include <string.h>

#include <kernel.h>
#include <libgs.h>

#include <modchip/io.h>
#include <modchip/apps.h>
#include "load_elf.h"
#include "crc32.h"

#ifndef NDEBUG
#include <sio.h>
#include "sio_ext.h"

#define print_debug(str) sio_puts(str)
#else
#define print_debug(str)
#endif

#define USER_MEM_START_ADDR 0x100000
#define USER_MEM_END_ADDR 0x2000000
#define USER_MEM_SIZE (USER_MEM_END_ADDR - USER_MEM_START_ADDR)

DISABLE_PATCHED_FUNCTIONS();

inline static void panic(const char *msg)
{
    print_debug(msg);
    GS_SET_BGCOLOR(0x70, 0x00, 0x00);
    asm volatile("break\n");
}

void __attribute__((section(".entry"))) __start()
{
    print_debug("EE2: clear");
    memset((void *)USER_MEM_START_ADDR, 0, USER_MEM_SIZE);

    u32 ee_stage3_size;
    if (!modchip_apps_read(MODCHIP_APPS_SIZE_OFFSET, MODCHIP_APPS_SIZE_SIZE, 1, &ee_stage3_size))
        panic("EE2: size read failed");

    if (!modchip_apps_read(MODCHIP_APPS_DATA_OFFSET, ee_stage3_size, 1, (void *)EE_STAGE_3_ADDR))
        panic("EE2: data read failed");

    u32 ee_stage3_crc;
    if (!modchip_apps_read(MODCHIP_APPS_CRC_OFFSET(ee_stage3_size), MODCHIP_APPS_CRC_SIZE, 1, &ee_stage3_crc))
        panic("EE2: crc read failed");

    u32 crc = crc32((void *)EE_STAGE_3_ADDR, ee_stage3_size);
#ifndef NDEBUG
    sio_putsn("EE2: read 0x");
    sio_putxn(crc);
    if (crc == ee_stage3_crc) sio_puts(" ok");
#endif
    if (crc != ee_stage3_crc) panic(" bad");

    print_debug("EE2: init");
    _InitSys();

    print_debug("EE2: exec");
    static char *argv[2];
    argv[0] = "payload";
    ExecPS2FromMemory((void *)EE_STAGE_3_ADDR, 1, argv);
}