#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <tamtypes.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <loadfile.h>
#include "sbv_patches.h"
#include <gsKit.h>

#include "modchip/flash.h"
#include "modchip/settings.h"

#include "masswatch.h"

#include "components/font.h"
#include "input/pad.h"
#include "scene/boot_list.h"
#include "scene/init.h"
#include "scene/superscene.h"
#include "apps.h"
#include "constants.h"
#include "embedded_irx.h"
#include "init.h"
#include "utils.h"

static void autoboot_read_settings(struct state *state)
{
    u32 v;
    bool r = modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT, &v);
    if (r) state->autoboot = v;
    else state->autoboot = false;
    printf("MODCHIP_SETTINGS_MENU_AUTOBOOT %u\n", state->autoboot);

    r = modchip_settings_get(MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY, &state->autoboot_delay_sec);
    if (!r) state->autoboot_delay_sec = 5;
    printf("MODCHIP_SETTINGS_MENU_AUTOBOOT_DELAY %u\n", state->autoboot_delay_sec);
}

static void init_gskit(struct state *state)
{
    state->gs = gsKit_init_global();
    state->gs->Width = 640;
    state->gs->Height = 448;

    state->gs->PSM = GS_PSM_CT32;
    state->gs->PSMZ = GS_PSMZ_16S;
    state->gs->DoubleBuffering = GS_SETTING_ON;
    state->gs->ZBuffering = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
            D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF); // initialize the DMAC

    gsKit_vram_clear(state->gs);
    gsKit_init_screen(state->gs);
    gsKit_display_buffer(state->gs); // switch display buffer to avoid garbage appearing on screen
    gsKit_TexManager_init(state->gs);

    font_init(state->gs);

    gsKit_set_primalpha(state->gs, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);
    state->gs->PrimAlphaEnable = GS_SETTING_ON;
    gsKit_mode_switch(state->gs, GS_ONESHOT);
}

int main()
{
    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {};
    while (!SifIopSync()) {};

    SifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();

    int ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    printf("SifExecModuleBuffer sioman res=%i\n", ret);
    if (ret < 0) SleepThread();

    // padman dependencies: loadcore, intrman, stdio, sio2man, thbase, thevent, sifman, sifcmd, vblank
    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    printf("SifExecModuleBuffer padman res=%i\n", ret);
    if (ret < 0) SleepThread();

    // start pads as early as possible, it takes a moment for them to connect and be usable
    input_pad_start();

    // enable loading IOP modules from EE RAM
    sbv_patch_enable_lmb();

    // enabling FWFS loading for elf-loader
    sbv_patch_disable_prefix_check();

    int mod_res = 0;
#ifndef NDEBUG
    ret = SifExecModuleBuffer(PPCTTY_IRX, PPCTTY_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer ppctty res=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();
#endif

    ret = SifExecModuleBuffer(FWFS_IRX, FWFS_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer fwfs ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(BDM_IRX, BDM_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer bdm ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(MASSWATCH_IRX, MASSWATCH_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer masswatch ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(BDMFS_FATFS_IRX, BDMFS_FATFS_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer bdmfs_fatfs ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(USBD_IRX, USBD_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer usbd ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(USBMASS_BD_IRX, USBMASS_BD_IRX_SIZE, 0, NULL, &ret);
    printf("SifExecModuleBuffer usbmass_bd ret=%i mod_res=%i\n", ret, mod_res);
    if (ret < 0) SleepThread();

    init_osd_config();

    masswatch_init();

    struct state *state = malloc(sizeof(*state));

    autoboot_read_settings(state);
    init_gskit(state);

    // skip initialization screen if controller is not connected
    if (input_pad_is_ready())
        scene_switch_to_boot_list(state);
    else
        scene_switch_to_init(state);

    int input, prevInput = 0;

    printf("entering main loop\n");
    while (true)
    {
        scene_tick_handler_superscene(state);

        input = input_pad_poll();
        if (input != prevInput)
        {
            scene_input_handler_superscene(state, input & ~prevInput);
            prevInput = input;
        }

        scene_paint_handler_superscene(state);
        gsKit_vsync_wait();
    }
}
