#include "stdio.h"

#include "kernel.h"
#include "sifrpc.h"
#include <sifcmd.h>

#include "masswatch.h"
#include "masswatch-sifcmd.h"

#define SIFCMD_BUFFER_SIZE 1

static SifCmdHandlerData_t sifcmd_buffer[SIFCMD_BUFFER_SIZE];

static connect_event_callback_t *connect_event_callback = NULL;
static void *connect_event_callback_arg = NULL;

static void mount_event_handler(void *p, void *harg)
{
    SifCmdHeader_t *packet = p;
    (void)harg;

    u8 is_mount_event = packet->opt;
    if (connect_event_callback) connect_event_callback(is_mount_event, connect_event_callback_arg);

    ExitHandler();
}

void masswatch_init()
{
    DI();
    sceSifSetCmdBuffer(sifcmd_buffer, SIFCMD_BUFFER_SIZE);
    sceSifAddCmdHandler(MASSWATCH_SIF_CMD_SEND_MOUNT_EVENT, mount_event_handler, NULL);
    EI();
}

void masswatch_set_connect_callback(connect_event_callback_t *cb, void *arg)
{
    connect_event_callback = cb;
    connect_event_callback_arg = arg;
}
