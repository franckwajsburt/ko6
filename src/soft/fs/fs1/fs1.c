/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/fs1/fs1.c
  \author   Franck Wajsburt
  \brief    Minimalist Read Only File System 1 directory --> FS1

  0   1   2   3   4   5   6   7   8   9  ... LBA (1 block = 4 kB)
  ┌───┌───────────┌───────┌───────────────┐
  │DIR│   app1.x  │app2.x │     app3.x    │  disk image built
  └───└───────────└───────└───────────────┘
      ┌─────────────────┐
  DIR:│  0: <unused>    │ name[24],LBA,size
      │  1:app1.x 1 11kB│
      │  2:app2.x 4 7kB │
      │  3.app3.x 6 15kB│
      │...:...... . ....│
      │127:             │ 127 file descriptors
      └─────────────────┘                                                     https://asciiflow.com
\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

//--------------------------------------------------------------------------------------------------
// Internal Stuctures & functions
//--------------------------------------------------------------------------------------------------

#define FS1_MAX_FILES   128
#define FS1_NAME_LEN    24

/** \brief   fs1 file metadata, that is actually the real file inode
 */
typedef struct fs1_inode_s {
    char name[FS1_NAME_LEN];                                ///< file name
    unsigned lba;                                           ///< first logical block 
    unsigned size;                                          ///< file size
} fs1_inode_t;

/** \brief  fs1 volume metadata
 */
typedef struct fs1_volume_s {
    fs1_inode_t *entries;                                   ///< metadata
    unsigned    entry_count;                                ///< maximum number of files
    unsigned    minor;                                      ///< block device minor number
} fs1_volume_t;
 
/** 
 * \brief get the volume (disk) of the superblock
 * \param sb super block
 * \return a pointer to the volume
 */
static fs1_volume_t *fs1_get_volume (const superblock_t *sb) 
{
    return (fs1_volume_t *) sb->fs_data;
}

/**
 * \brief Retrieve the pointer to the fs1 inode structure from its ino.
 * \param sb  Pointer to the superblock.
 * \param ino Index of the inode in the volume table.
 * \return Pointer to the fs1_inode_t entry if valid, NULL otherwise.
 */
static fs1_inode_t *fs1_retrieve_inode (const superblock_t *sb, ino_t ino) 
{
    fs1_volume_t *vol = fs1_get_volume (sb);
    return (ino < vol->entry_count) ? &vol->entries[ino] : NULL;
}

/**
 * \brief Create a VFS inode from a real fs1 inode.
 * \param sb Pointer to the superblock.
 * \param ino Index of the real inode in the fs1 volume.
 * \return A pointer to the allocated vfs_inode_t, or NULL on error.
 * \note Index 0 is reserved for the synthetic root inode and should not be used for regular files
 */
static vfs_inode_t *fs1_new_inode (superblock_t *sb, ino_t ino) 
{
    const fs1_inode_t *entry = fs1_retrieve_inode (sb, ino);// retrieve the fs1_inode from the ino
    if (!entry) return NULL;

    size_t size = (ino) ? entry->size : BLOCK_SIZE;         // special case for the root directory
    mode_t mode = (ino) ? S_IFREG : S_IFDIR;                // ino 0 --> DIR, else REGular file 
    mode       |= S_IROTH|S_IXOTH|S_IRUSR|S_IXUSR;          // All can read and execute
    void *data  = (ino) ? (void *)entry : NULL;             // fs1_inode itself

    return vfs_inode_create (sb, ino, size, mode, data);    // finally create the vfs_inode
}

//--------------------------------------------------------------------------------------------------
// Physical File System API, function signatures are documented in fs/vfs.h
//--------------------------------------------------------------------------------------------------

static int fs1_mount (superblock_t *sb, blockdev_t *bdev)     
{
    fs1_volume_t *vol = kmalloc (sizeof (fs1_volume_t));    // create a new volume
    if (!vol) return -ENOMEM;                               // return if no memory
    vol->entries = blockio_get (bdev->minor, 0);            // read the disk metadata (first block)
    if (!vol->entries) { kfree (vol); return -EIO; }        // return if impossible to read disk
    page_set_lock (vol->entries);                           // lock the metada block page

    vol->entry_count = FS1_MAX_FILES;                       // Maximum number of files
    vol->minor = bdev->minor;                               // block device identifier

    sb->bdev = bdev;                                        // real block device
    sb->ops = &fs1_ops;                                     // API implementation
    sb->fs_data = vol;                                      // real file system

    sb->root = fs1_new_inode (sb, 0);                       // inode root of the superblock
    if (!sb->root) {                                        // no more memory space
        page_clr_lock (vol->entries);                       // unlock the metadata page
        blockio_release (vol->entries);                     // release the block
        kfree (vol);                                        // volume no longer needed (cleanup)
        return -ENOMEM;                                     // return the error
    }
    return 0;                                               // success
}

static int fs1_unmount (superblock_t *sb)
{
    (void)sb;
    return -ENOSYS;
}

/**
 * \note Inode reference counting model:
 * 
 * When a new vfs_inode_t is created (e.g., by fs1_new_inode ()), its reference count (refcount) 
 * is set to 1. This 1st reference represents the presence of the inode in memory it is in the VFS.
 *
 * When an inode is actively used (for example by lookup, open, or other operations),
 * the function vfs_inode_get() must be called to increment the refcount.
 * Each user must hold its own reference to the inode while it uses it.
 *
 * Therefore, after fs1_lookup() creates a new inode and calls vfs_inode_get(),
 * the refcount becomes 2:
 *   - One reference for the inode being stored in memory (created).
 *   - One reference for the active usage by the lookup (or open).
 *
 * When the user is finished (e.g., after vfs_close()), it must call vfs_inode_release(),
 * decrementing the refcount.
 * When the refcount reaches zero, the inode is automatically freed from memory.
 *
 * This model ensures safe sharing and lifetime management of inodes,
 * without shortcuts or hidden dependencies.
 */
static vfs_inode_t *fs1_lookup (superblock_t *sb, vfs_inode_t *dir, const char *name) 
{
    (void)dir;
    fs1_volume_t *vol = fs1_get_volume (sb);                // volume of the superblock
    for (unsigned i = 0; i < vol->entry_count; ++i) {       // for all possible files in dir
        char *curname = vol->entries[i].name;               // get the current name
        if (strncmp (name, curname, FS1_NAME_LEN) == 0) {   // check if name is found
            vfs_inode_t *inode = vfs_inode_lookup (sb, i);  // if yes lookup the vfs_inode
            if (inode) {                                    // if found
                vfs_inode_get (inode);                      // incrément it
                return inode;                               // return it
            }
            inode = fs1_new_inode (sb, i);                  // if not found create it refcount <- 1
            if (inode) {                                    // if success 
                vfs_inode_get (inode);                      // then refcount <- 2
            }
            return inode;
        }
    }
    return NULL;
}

static int fs1_read (vfs_inode_t *inode, void *buffer, unsigned offset, unsigned size) 
{
    fs1_inode_t *ent = inode->data;
    if (offset >= ent->size) return 0;
    if (offset + size > ent->size) size = ent->size - offset;

    unsigned start_lba = ent->lba + offset / BLOCK_SIZE;
    unsigned end_lba   = ent->lba + (offset + size - 1) / BLOCK_SIZE;
    unsigned lba_offset = offset % BLOCK_SIZE;
    unsigned copied = 0;

    unsigned minor = inode->sb->bdev->minor; // see header of fs/vfs.h to get an explanation

    for (unsigned lba = start_lba; lba <= end_lba; lba++) {
        void *page = blockio_get (minor, lba);
        if (!page) return copied ? copied : -EIO;

        unsigned page_offset = (lba == start_lba) ? lba_offset : 0;
        unsigned to_copy = BLOCK_SIZE - page_offset;
        if (to_copy > size - copied) to_copy = size - copied;

        memcpy ((char *)buffer + copied, (char *)page + page_offset, to_copy);
        copied += to_copy;

        blockio_release (page);
    }
    return copied;
}

static int fs1_write (vfs_inode_t *inode, const void *buffer, unsigned offset, unsigned size)
{
    (void)inode;
    (void)buffer;
    (void)offset;
    (void)size;
    return -ENOSYS;
}

static vfs_inode_t *fs1_create (vfs_inode_t *dir, const char *name, unsigned mode)
{
    (void)dir;
    (void)name;
    (void)mode;
    return NULL; 
}

static vfs_inode_t *fs1_mkdir (vfs_inode_t *dir, const char *name, mode_t mode)
{
    (void)dir;
    (void)name;
    (void)mode;
    return NULL; 
}

static int fs1_unlink (vfs_inode_t *dir, const char *name)
{
    (void)dir;
    (void)name;
    return -ENOSYS;
}

static int fs1_readdir (vfs_inode_t *dir, unsigned index, char *name, unsigned maxlen)
{
    (void)dir;
    (void)index;
    (void)name;
    (void)maxlen;
    return -ENOSYS;
}

static int fs1_getattr (vfs_inode_t *inode, struct stat *stbuf)
{
    (void)inode;
    (void)stbuf;
    return -ENOSYS;
}

static int fs1_setattr (vfs_inode_t *inode, const struct stat *stbuf)
{
    (void)inode;
    (void)stbuf;
    return -ENOSYS;
}

/**
 * \brief VFS operation table for the fs1 filesystem.
 *        Only a subset of the VFS operations is implemented.
 *        Unsupported operations are set to NULL.
 */
vfs_fs_type_t fs1_ops = {
    .name     = "fs1"       ,
    .mount    = fs1_mount   ,
    .unmount  = fs1_unmount ,   // not used with fs1
    .lookup   = fs1_lookup  ,
    .read     = fs1_read    ,
    .write    = fs1_write   ,   // not used with fs1
    .create   = fs1_create  ,   // not used with fs1
    .mkdir    = fs1_mkdir   ,   // not used with fs1
    .unlink   = fs1_unlink  ,   // not used with fs1
    .readdir  = fs1_readdir ,   // not used with fs1
    .getattr  = fs1_getattr ,   // not used with fs1
    .setattr  = fs1_setattr     // not used with fs1
};

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
*------------------------------------------------------------------------------------------------*/
