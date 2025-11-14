#include <stdio.h>
#include <stdarg.h>
#include "errno.h"

#include <tamtypes.h>
#include <sysclib.h>
#include <sysmem.h>
#include <excepman.h>
#include <intrman.h>
#include <ioman.h>
#include <thsemap.h>
#include "loadcore.h"

#include "fwfs.h"
#include <modchip/io.h>
#include <modchip/apps.h>
#include <modchip/fwfs.h>
#include "minmax.h"

#define MAX_FILES 8

typedef struct
{
    char mode;
    u8 idx;
    u32 size; // note: fwfs_open relies on `size` and `attr` placed next to each other
    u32 attr;
    u32 offset;
} fwfs_file_t;

static fwfs_file_t fd_handle[MAX_FILES];

static int fwfs_sema = -1;

static fwfs_file_t *find_free_file_handle(void)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (!fd_handle[i].size)
        {
            return &fd_handle[i];
        }
    }

    return NULL;
}

static fwfs_file_t *get_file_handle_by_idx(u8 idx)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (fd_handle[i].size && fd_handle[i].idx == idx)
        {
            return &fd_handle[i];
        }
    }

    return NULL;
}

static u8 get_idx_from_name(const char *name)
{
    if (name[0] == '/')
        return name[2];
    else
        return name[1];
}

static char get_mode_from_name(const char *name)
{
    if (name[0] == '/')
        return name[1];
    else
        return name[0];
}


static void fs_lock(void)
{
    DPRINTF("lock\n");
    WaitSema(fwfs_sema);
}

static void fs_unlock(void)
{
    DPRINTF("unlock\n");
    SignalSema(fwfs_sema);
}

static int fwfs_not_implemented() { return -ENOSYS; }

static int fwfs_init(iop_device_t *device)
{
    (void)device;

    DPRINTF("init\n");

    if ((fwfs_sema = CreateMutex(IOP_MUTEX_UNLOCKED)) < 0)
    {
        DPRINTF("failed to create mutex\n");
        return -ENOLCK;
    }

    return 0;
}

static int fwfs_deinit(iop_device_t *device)
{
    (void)device;

    DPRINTF("deinit\n");
    DeleteSema(fwfs_sema);

    return 0;
}

static int fwfs_open(iop_file_t *file, const char *name, int flags)
{
    (void)file;
    (void)name;
    (void)flags;

    DPRINTF("open: name=%s flags=%i\n", name, flags);

    fs_lock();

    fwfs_file_t *fd = find_free_file_handle();
    if (fd == NULL)
    {
        fs_unlock();
        return -EMFILE;
    }

    fd->idx = get_idx_from_name(name);
    fd->mode = get_mode_from_name(name);

    if (fd->mode == FWFS_MODE_DATA_CHAR)
    {
        s32 ret = modchip_apps_read(MODCHIP_APPS_SIZE_OFFSET, sizeof(fd->size) + sizeof(fd->attr), fd->idx, &fd->size, true);
        DPRINTF("open: data: result %i\n", ret);
        if (ret)
        {
            fs_unlock();
            return -EIO;
        }

        fd->offset = MODCHIP_APPS_DATA_OFFSET;
    }
    else if (fd->mode == FWFS_MODE_ATTR_CHAR)
    {
        s32 ret = modchip_apps_read(MODCHIP_APPS_ATTR_SIZE, sizeof(fd->attr), fd->idx, &fd->attr, true);
        DPRINTF("open: attr: result %i\n", ret);
        if (ret)
        {
            fs_unlock();
            return -EIO;
        }

        fd->size = MODCHIP_APPS_ATTR_SIZE;
        fd->offset = MODCHIP_APPS_ATTR_OFFSET;
    }
    else
    {
        fs_unlock();
        return -ENOSYS;
    }

    file->privdata = fd;

    DPRINTF("open: idx=%i size=%i attr=0x%x\n", fd->idx, fd->size, fd->attr);

    fs_unlock();

    return 0;
}

static int fwfs_close(iop_file_t *file)
{
    (void)file;

    DPRINTF("close\n");

    fwfs_file_t *fd = (fwfs_file_t *)file->privdata;
    if (fd == NULL)
        return -EBADF;

    fs_lock();

    fd->idx = 0;
    fd->size = 0;
    fd->attr = 0;
    fd->offset = 0;

    fs_unlock();

    return 0;
}

static int fwfs_read(iop_file_t *file, void *ptr, int size)
{
    (void)file;

    DPRINTF("read: ptr=%p size=%i\n", ptr, size);

    fs_lock();

    fwfs_file_t *fd = (fwfs_file_t *)file->privdata;
    if (fd == NULL)
    {
        fs_unlock();
        return -EBADF;
    }

    if (fd->offset >= fd->size + MODCHIP_APPS_DATA_OFFSET)
    {
        fs_unlock();
        return 0; // EOF
    }

    u32 remaining_size = fd->size - (fd->offset - MODCHIP_APPS_DATA_OFFSET);
    u32 available_size = MIN((u32)size, remaining_size);
    DPRINTF("read: rem %i ava %i\n", remaining_size, available_size);

    s32 ret = modchip_apps_read(fd->offset, available_size, fd->idx, ptr, true);
    DPRINTF("read: cmd: off %i size %i idx %i: ret %i\n", fd->offset, size, fd->idx, ret);
    if (ret)
    {
        fs_unlock();
        return -EIO;
    }

    fd->offset += available_size;

    DPRINTF("read: ok %i offset %i\n", available_size, fd->offset);
    fs_unlock();

    return available_size;
}

static int fwfs_getstat(iop_file_t *file, const char *name, io_stat_t *stat)
{
    (void)file;

    DPRINTF("stat: name=%s\n", name);

    fs_lock();

    u8 idx = get_idx_from_name(name);
    fwfs_file_t *fd = get_file_handle_by_idx(idx);
    DPRINTF("stat: idx=%i fd=%p\n", idx, fd);

    if (fd == NULL)
    {
        fwfs_file_t file = {};

        s32 ret = modchip_apps_read(MODCHIP_APPS_SIZE_OFFSET, sizeof(fd->size) + sizeof(fd->attr), idx, &file.size, true);
        DPRINTF("stat: result %i\n", ret);
        if (ret)
        {
            fs_unlock();
            return -EIO;
        }

        stat->size = file.size;
    }
    else
    {
        stat->size = fd->size;
    }

    fs_unlock();
    return 0;
}

static int fwfs_lseek(iop_file_t *file, int offset, int whence)
{
    DPRINTF("lseek: offset=%i whence=%i\n", offset, whence);

    fs_lock();

    fwfs_file_t *fd = (fwfs_file_t *)file->privdata;
    if (fd == NULL)
    {
        fs_unlock();
        return -EBADF;
    }

    switch (whence)
    {
        case SEEK_SET:
            fd->offset = offset + MODCHIP_APPS_DATA_OFFSET;
            break;
        case SEEK_CUR:
            fd->offset += offset;
            break;
        case SEEK_END:
            fd->offset = (fd->size + MODCHIP_APPS_DATA_OFFSET) - offset;
            break;
        default:
            fs_unlock();
            return -EINVAL;
    }

    if ((s32)fd->offset < 0)
        fd->offset = MODCHIP_APPS_DATA_OFFSET;

    if (fd->offset > fd->size + MODCHIP_APPS_DATA_OFFSET)
        fd->offset = fd->size + MODCHIP_APPS_DATA_OFFSET;

    fs_unlock();
    return fd->offset - MODCHIP_APPS_DATA_OFFSET;
}

static iop_device_ops_t fsd_ops =
{
    &fwfs_init,
    &fwfs_deinit,
    (void *)&fwfs_not_implemented,
    fwfs_open,
    fwfs_close,
    fwfs_read,
    (void *)&fwfs_not_implemented,
    fwfs_lseek,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    (void *)&fwfs_not_implemented,
    fwfs_getstat,
    (void *)&fwfs_not_implemented,
};

static iop_device_t fwfs_fsd =
{
    "fwfs",
    IOP_DT_FS,
    1,
    "Modchip FWFS",
    &fsd_ops,
};

int init_fwfs(void)
{
    DelDrv(fwfs_fsd.name);

    if (AddDrv(&fwfs_fsd) != 0)
        return -1;

    return MODULE_RESIDENT_END;
}
