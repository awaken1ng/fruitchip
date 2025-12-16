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

    print_debug("EE2: init");
    _InitSys();

    print_debug("EE2: exec");
    static char *argv[1] = { "" };
    if (!ExecPS2FromModchipStage3(1, argv))
        panic("EE2: exec fail");
}