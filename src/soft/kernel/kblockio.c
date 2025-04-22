/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-19
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kblockio.c
  \author   Franck Wajsburt
  \brief    Block I/O cache and access layer for disk devices.

\*------------------------------------------------------------------------------------------------*/

#include <kernel/kblockio.h>
#include <kernel/klibc.h>

void *blockio_get (unsigned bdev, unsigned lba) 
{
    blockdev_t *dev = blockdev_get (bdev);

#if 0
VAR(%p\t\n,dev->base);
VAR(%d\t\n,dev->blocks);
VAR(%d\n,dev->block_size);
VAR(%d\t\n,dev->ppb);
VAR(%p\n,dev->event.fn);
VAR(%p\n,dev->event.arg);
#endif
        
    if (!dev || !dev->ops || !dev->ops->blockdev_read) return NULL;

    void *page = kmalloc (PAGE_SIZE);
    if (!page) return NULL;
    page_set_lba (page, bdev, lba);
    page_set_block (page);
    page_inc_refcount (page);
    page_set_valid (page);

    if (dev->ops->blockdev_read (dev, lba, page, 1) != 0) { 
        kfree (page);
        return NULL;
    }
    return page;
}

int blockio_release (void *page)
{
    int err = 0;
    if (page_get_refcount (page) == 1) {
        err = blockio_sync (page);
    }

    if (page_dec_refcount (page) == 0) {
        kfree (page);
    }
    return err;
}

int blockio_sync (void *page) 
{
    if (!page) return -EINVAL;

    if (!page_is_dirty (page)) return 0;

    unsigned bdev, lba;
    page_get_lba (page, &bdev, &lba);

    blockdev_t *dev = blockdev_get (bdev);
    if (!dev || !dev->ops || !dev->ops->blockdev_write) return -EIO;

    int err = dev->ops->blockdev_write (dev, lba, page, 1);
    page_clear_dirty (page);

    return err;
}

typedef struct { 
    char     name[24];   // filename 23 bytes + '\0' 
    unsigned lba;        // logical block position 
    unsigned size;       // size if bytes 
} entry_t; 

void blockio_init (void)
{
    entry_t *dir = blockio_get (0, 0);
    for (int e=0 ; e < 128 && dir[e].name; e++) {
        if (dir[e].name[0])
            kprintf ("   [%d]\t(%d)\t%s\n", dir[e].lba, dir[e].size, dir[e].name);
    }
}
/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
