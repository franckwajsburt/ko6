/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-21
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     fs/fs1/fs1.c
  \author   Franck Wajsburt
  \brief    Minimalist Read Only File System 1 directory --> FS1

  0   1   2   3   4   5   6   7   8   9  ... LBA (1 block = 4 kB)
  ┌───┌───────────┌───────┌───────────────┐
  │DIR│   app1.x  │app2.x │     app3.x    │  disk image built
  └───└───────────└───────└───────────────┘
      ┌─────────────────┐
  DIR:│  0:             │ name[24],LBA,size
      │  1:app1.x 1 11kB│
      │  2:app2.x 4 7kB │
      │  3.app3.x 6 15kB│
      │...:...... . ....│
      │127:             │ 128 file descr.
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
static fs1_volume_t *fs1_get_vol(const superblock_t *sb) 
{
    return (fs1_volume_t *) sb->fs_data;
}

/**
 * \brief Retrieve the pointer to the fs1 inode structure from its index.
 * \param sb    Pointer to the superblock.
 * \param index Index of the inode in the volume table.
 * \return Pointer to the fs1_inode_t entry if valid, NULL otherwise.
 */
static fs1_inode_t *fs1_get_inode (const superblock_t *sb, unsigned index) 
{
    fs1_volume_t *vol = fs1_get_vol(sb);
    return (index < vol->entry_count) ? &vol->entries[index] : NULL;
}

/**
 * \brief Create a VFS inode from a real fs1 inode.
 * \param sb    Pointer to the superblock.
 * \param index Index of the real inode in the fs1 volume.
 * \return A pointer to the allocated vfs_inode_t, or NULL on error.
 * \note Index 0 is reserved for the synthetic root inode and should not be used for regular files
 *       FIXME This function create a vfs_inode because there is no vfs_inode cache yet 
 */
static vfs_inode_t *fs1_create_inode (superblock_t *sb, unsigned index)
{
    fs1_inode_t *entry = fs1_get_inode (sb, index);         // retreive the fs1_inode from the index
    if (!entry) return NULL;
    vfs_inode_t *ino = kmalloc (sizeof(vfs_inode_t));       // allocate a new vfs_inode
    if (!ino) return NULL;

    ino->sb = sb;                                           // superblock to know thi ino come from
    ino->index = index;                                     // real inode indentifier (for this fs)
    ino->size  = (index) ? entry->size : BLOCK_SIZE;        // specical case for the root directory
    ino->mode |= (index) ? S_IFREG : S_IFDIR;               // index 0 --> DIR, else REGular file 
    ino->mode  = S_IROTH|S_IXOTH|S_IRUSR|S_IXUSR;           // All can read and execute
    ino->data  = (void *)entry;                             // fs1_inode itself
    ino->refcount = 1;                                      // new vfs_inode so use once

    return ino;
}

//--------------------------------------------------------------------------------------------------
// Physical File System API, function signatures are documented in fs/vfs.h
//--------------------------------------------------------------------------------------------------

static int fs1_mount (superblock_t *sb, blockdev_t *bdev)     
{
    fs1_volume_t *vol = kmalloc (sizeof (fs1_volume_t));    // create a new volume
    if (!vol) return -ENOMEM;                               // return if no memmory
    vol->entries = blockio_get (bdev->minor, 0);            // read the disk metadata (first block)
    if (!vol->entries) { kfree (vol); return -EIO; }        // return if impossible to read disk
    page_set_lock (vol->entries);                           // lock the metada block page

    vol->entry_count = FS1_MAX_FILES;                       // Maximum number of files
    vol->minor = bdev->minor;                               // block device identifier

    sb->bdev = bdev;                                        // real block device
    sb->ops = &fs1_ops;                                     // API implementation
    sb->fs_data = vol;                                      // real file system

    sb->root = fs1_create_inode (sb, 0);                    // inode root of the superblock
    if (!sb->root) {                                        // no more memory space
        page_clr_lock (vol->entries);                         // unlock the metadata papge
        blockio_release (vol->entries);                     // release the block
        kfree (vol);                                        // volume is unuseful
        return -ENOMEM;                                     // return the error
    }
    return 0;                                               // success
}

static vfs_inode_t *fs1_lookup (superblock_t *sb, vfs_inode_t *dir, const char *name) 
{
    fs1_volume_t *vol = fs1_get_vol (sb);                   // volume of the superblock
    for (unsigned i = 0; i < vol->entry_count; ++i) {       // for all possible files 
        char * curname = vol->entries[i].name;              // get the current name
        if (strncmp (name, curname, FS1_NAME_LEN) == 0) {   // check if name is found
            return fs1_create_inode (sb, i);                // if yes return the inode
        }
    }
    return NULL;                                            // or NULL on failure
}

static int fs1_read (vfs_inode_t *ino, void *buffer, unsigned offset, unsigned size) 
{
    const fs1_inode_t *ent = (const fs1_inode_t *)ino->data;
    if (offset >= ent->size) return 0;
    if (offset + size > ent->size) size = ent->size - offset;

    unsigned start_lba = ent->lba + offset / BLOCK_SIZE;
    unsigned end_lba   = ent->lba + (offset + size - 1) / BLOCK_SIZE;
    unsigned lba_offset = offset % BLOCK_SIZE;
    unsigned copied = 0;

    unsigned minor = ino->sb->bdev->minor; // see header of fs/vfs.h to get an explanation

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

/**
 * \brief VFS operation table for the fs1 filesystem.
 *        Only a subset of the VFS operations is implemented.
 *        Unsupported operations are set to NULL.
 */
struct fs_ops_s fs1_ops = {
    .mount    = fs1_mount,
    .unmount  = NULL,
    .lookup   = fs1_lookup,
    .read     = fs1_read,
    .write    = NULL,
    .create   = NULL,
    .mkdir    = NULL,
    .unlink   = NULL,
    .readdir  = NULL,
    .getattr  = NULL,
    .setattr  = NULL
};

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
*------------------------------------------------------------------------------------------------*/
