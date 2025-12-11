#include <git_version.h>

#include <modchip/version.h>

static char FW_GIT_REV[9] = "N/A";
static char BOOTLOADER_GIT_REV[9] = "N/A";

bool version_init_firmware()
{
    if (modchip_fw_git_rev(FW_GIT_REV))
    {
        return true;
    }
    else
    {
        FW_GIT_REV[0] = 'N';
        FW_GIT_REV[1] = '/';
        FW_GIT_REV[2] = 'A';
        FW_GIT_REV[3] = 0;
        FW_GIT_REV[4] = 0;
        FW_GIT_REV[5] = 0;
        FW_GIT_REV[6] = 0;
        FW_GIT_REV[7] = 0;
        FW_GIT_REV[8] = 0;
        return false;
    }
}

bool version_init_bootloader()
{
    if (modchip_bootloader_git_rev(BOOTLOADER_GIT_REV))
    {
        return true;
    }
    else
    {
        BOOTLOADER_GIT_REV[0] = 'N';
        BOOTLOADER_GIT_REV[1] = '/';
        BOOTLOADER_GIT_REV[2] = 'A';
        BOOTLOADER_GIT_REV[3] = 0;
        BOOTLOADER_GIT_REV[4] = 0;
        BOOTLOADER_GIT_REV[5] = 0;
        BOOTLOADER_GIT_REV[6] = 0;
        BOOTLOADER_GIT_REV[7] = 0;
        BOOTLOADER_GIT_REV[8] = 0;
        return false;
    }
}

const char *version_get_menu()
{
    return GIT_REV;
}

const char *version_get_firmware()
{
    return FW_GIT_REV;
}

const char *version_get_bootloader()
{
    return BOOTLOADER_GIT_REV;
}
