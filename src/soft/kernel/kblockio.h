/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-19
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kblockio.h
  \author   Franck Wajsburt
  \brief    Block I/O cache and access layer for disk devices.

            This API provides access to logical blocks on block devices,
            with transparent caching, reference counting, and delayed write-back.
            A block is always of fixed size (one page size)
            The returned page is page-aligned and must be released explicitly.

\*------------------------------------------------------------------------------------------------*/

#ifndef _KBLOCKIO_H_
#define _KBLOCKIO_H_

#include <hal/devices/blockdev.h>

/**
 * \brief   Get a page for the given logical block.
 *          If the block is already cached, returns a pointer to the cached page.
 *          Otherwise, allocates a new page and loads the block from the device.
 *          The page's reference count is incremented.
 * \param   bdev Block device identifier
 * \param   lba Logical Block Address (in units of BLOCK_SIZE)
 * \return  Pointer to the page in memory, or NULL on error
 */
void *blockio_get(unsigned bdev, unsigned lba);

/**
 * \brief Mark a page (used as block) as dirty (modified).
 *        This signals that the page has been changed and should be written
 *        back to disk at some point (e.g. during sync or eviction).
 * \param page Pointer to the page returned by blockio_get()
 */
void blockio_dirty(void *page);

/**
 * \brief Mark a page (used as block) as locked.
 *        This signals that the block cannot be evicted, even if the reference count is 0
 * \param page Pointer to the page returned by blockio_get()
 */
void blockio_lock(void *page);

/**
 * \brief Mark a page (used as block) as unlocked.
 *        This signals that the block can be evicted
 * \param page Pointer to te page returned by blockio_get()
 */
void blockio_unlock(void *page);

/**
 * \brief Release a previously acquired block page.
 *        Ask to synchronize the page if 
 *        Decrements the reference count. The page remains in the cache
 *        and may be reused or evicted later depending on usage.
 * \param page Pointer to the page to release
 * \return 0 on success, -EIO or -EINVALL on fealure
 */
int blockio_release(void *page);

/**
 * \brief Force the write-back of a specific page to disk, it is dirty.
 *        If the block page is clean or null, does nothing
 * \param page Pointer to the page to sync
 * \return 0 on success, -EIO or -EINVALL on fealure
 */
int blockio_sync(void *page);

/**
 * \brief Write back all dirty blocks in the cache.
 */
void blockio_flush(void);

/**
 * \brief initialize the blockio layer for all block devices
 */
void blockio_init(void);

#endif//_KBLOCKIO_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
