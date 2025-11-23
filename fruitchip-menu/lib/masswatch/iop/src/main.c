#include "stdbool.h"

#include "loadcore.h"

#include "masswatch-sifcmd.h"
#include "irx_imports.h"

#define MODNAME "masswatch"

#ifndef NDEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

IRX_ID(MODNAME, 1, 0);

static void bdm_callback(int mounted)
{
    DPRINTF("%s mounted=%i\n", __func__, mounted);

    SifCmdHeader_t packet;
    packet.opt = mounted;
    unsigned int id = sceSifSendCmd(MASSWATCH_SIF_CMD_SEND_MOUNT_EVENT, &packet, sizeof(packet), NULL, NULL, 0);
    while (sceSifDmaStat(id) >= 0);
}

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    sceSifInitCmd();

    bdm_RegisterCallback(bdm_callback);

    DPRINTF("registered\n");

    return MODULE_RESIDENT_END;
}
