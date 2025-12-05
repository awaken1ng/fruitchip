#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "iopcontrol.h"
#include "iopheap.h"
#include "kernel.h"
#include "loadfile.h"
#include "sbv_patches.h"
#include "sifrpc-common.h"

#include "modchip/apps.h"
#include "modchip/flash.h"
#include "modchip/settings.h"
#include "modchip/fwfs.h"

#include "embedded_irx.h"
#include "test.h"
#include "utility.h"

static void run_tests()
{
    bool failed = false;
    failed |= !test_data_out();
    failed |= !test_settings();
    failed |= !test_apps();
    failed |= !test_menu();

    if (!failed)
    {
        print_combined("pass\n");
    }
    else
    {
        print_combined("fail\n");
    }
}

int main()
{
    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {};
    while (!SifIopSync()) {};

    SifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();

    // enable loading IOP modules from EE RAM
    sbv_patch_enable_lmb();

    // enabling FWFS loading for elf-loader
    sbv_patch_disable_prefix_check();

    int mod_res = 0;
    int ret = SifExecModuleBuffer(PPCTTY_IRX, PPCTTY_IRX_SIZE, 0, NULL, &mod_res);
    if (ret < 0) SleepThread();

    ret = SifExecModuleBuffer(FWFS_IRX, FWFS_IRX_SIZE, 0, NULL, &mod_res);
    if (ret < 0) SleepThread();

    init_scr();
    scr_printf("\n\n\n");

    run_tests();

    SleepThread();
}
