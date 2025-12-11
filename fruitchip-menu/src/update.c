#include <fcntl.h>
#include <unistd.h>

#include "update.h"

static int open_fw_update_file()
{
    return open("mass:/" FW_UPDATE_PATH, O_RDONLY);
}

static int open_apps_update_file()
{
    return open("mass:/" APPS_UPDATE_PATH, O_RDONLY);
}

int update_file_open(enum update_type ty)
{
    switch (ty)
    {
        case UPDATE_TYPE_FW:
            return open_fw_update_file();
        case UPDATE_TYPE_APPS:
            return open_apps_update_file();
    }
}

bool update_file_is_present(enum update_type ty)
{
    int fd = update_file_open(ty);

    if (fd >= 0)
    {
        close(fd);
        return true;
    }

    return false;
}
