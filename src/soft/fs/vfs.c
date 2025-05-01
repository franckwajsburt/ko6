/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-01
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.c
  \author   Franck Wajsburt
  \brief    Virtual File System

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

//--------------------------------------------------------------------------------------------------
// dentry API
//--------------------------------------------------------------------------------------------------

vfs_dentry_t *vfs_dentry_lookup (vfs_dentry_t *parent, const char *name)
{
    (void)parent;
    (void)name;
    return NULL;
}

vfs_dentry_t *vfs_dentry_create (vfs_dentry_t *parent, const char *name, vfs_inode_t *inode)
{
    (void)parent;
    (void)name;
    (void)inode;
    return NULL;
}

void vfs_dentry_destroy (vfs_dentry_t *dentry)
{
    (void)dentry;
}

//--------------------------------------------------------------------------------------------------
// inode cache API
//--------------------------------------------------------------------------------------------------

/**
 * \brief Insert an inode into the VFS inode cache.
 * \param inode Pointer to the vfs_inode_t to insert.
 * \note For now, this function does nothing because there is no inode cache yet.
 */
static void vfs_inode_cache_insert (vfs_inode_t *inode)
{
    (void)inode;
    // TODO: Implement inode cache insertion when cache will exist
}

/**
 * \brief Remove an inode from the VFS inode cache.
 * \param inode Pointer to the vfs_inode_t to remove.
 * \note For now, this function does nothing because there is no inode cache yet.
 */
static void vfs_inode_cache_remove (vfs_inode_t *inode)
{
    (void)inode;
    // TODO: Implement inode cache removal when cache will exist
}

extern vfs_inode_t *vfs_inode_create (superblock_t *sb, ino_t ino, size_t size, mode_t mode, void * data)
{
    vfs_inode_t *inode = kmalloc (sizeof(vfs_inode_t));     // allocate a new vfs_inode
    if (!inode) return NULL;

    inode->sb = sb;
    inode->ino = ino;
    inode->size = size;
    inode->mode = mode;
    inode->refcount = 1;
    inode->flags = 0;
    inode->data = data;
    inode->mapping = NULL;
    inode->dentries = NULL;
    list_init (&inode->list);

    return inode;
}

vfs_inode_t *vfs_inode_lookup (superblock_t *sb, ino_t ino)
{
    (void)sb;       // not used since no cache
    (void)ino;      // not used since no cache
    vfs_inode_cache_insert (NULL);
    return NULL;    // No cache yet
}

void vfs_inode_get (vfs_inode_t *inode)
{
    inode->refcount++;
}

void vfs_inode_release (vfs_inode_t *inode)
{
    if (--(inode->refcount) == 0) {
        vfs_inode_cache_remove (inode);
        kfree (inode);
    }
}

//--------------------------------------------------------------------------------------------------
// VFS API exposed to syscall layer
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------- mount / unmount

int vfs_mount (superblock_t *sb, blockdev_t *bdev, const struct fs_ops_s *ops)
{
    if (!sb || !bdev || !ops) return -EINVAL;               // check arguments validity
                                                            
    int ret = ops->mount (sb, bdev);                        // Call the fs-specific mount function
    if (ret == 0) {
        sb->ops = ops;                                      // assign the filesystem operations
        sb->bdev = bdev;                                    // assign the block device
        sb->fs_data = NULL;                                 // will be init by fs-specific mount
    }                                                        
    return ret;                                             // 0 on success, < 0 on failure
}

int vfs_umount (superblock_t *sb) 
{
    (void)sb;
    return -ENOSYS;
}

//---------------------------------------------------------------------------- lookup / open / close

vfs_inode_t *vfs_lookup (superblock_t *sb, vfs_inode_t *dir, const char *path)
{
    if (!sb || !sb->ops || !sb->ops->lookup)
        return NULL;

    return sb->ops->lookup (sb, dir, path);
}

vfs_file_t *vfs_open (superblock_t *sb, const char *path)
{
    if (!sb || !path) return NULL;                          // check arguments validity

    vfs_inode_t *root = sb->root;                           // Only support root dir lookup for now
    if (!root) return NULL;

    vfs_inode_t *inode = sb->ops->lookup (sb, root, path);  // Lookup the file
    if (!inode) return NULL;

    vfs_file_t *file = kmalloc (sizeof(vfs_file_t));        // Allocate a file descriptors
    if (!file) { 
        vfs_inode_release (inode);                          // Release the inode if allocation fails
        return NULL; 
    }

    vfs_inode_get (inode);
    file->inode = inode;
    file->offset = 0;

    return file;
}

int vfs_close (vfs_file_t *file)
{
    vfs_inode_release (file->inode);
    kfree (file);
    return 0;
}

//-------------------------------------------------------------------- read / write / seek / readdir

int vfs_read (vfs_file_t *file, void *buffer, unsigned size)
{
    if (!file || !buffer || !size) return -EINVAL;                    // check arguments validity

    vfs_inode_t *inode = file->inode;                                 // retreive file's vfs_inode
    if (!inode||!inode->sb||!inode->sb->ops||!inode->sb->ops->read)   // check structures 
        return -EINVAL;

    int ret = inode->sb->ops->read (inode, buffer, file->offset, size);// Perform the read
    if (ret < 0) return ret;

    file->offset += ret;                                               // Advance the file offset

    return ret;
}

int vfs_write (vfs_file_t *file, const void *buffer, unsigned size) 
{
    (void)file;
    (void)buffer;
    (void)size;
    return -ENOSYS;
}

int vfs_seek (vfs_file_t *file, int offset, int whence)
{
    unsigned size;
    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        return 0;
    case SEEK_CUR:
        file->offset += offset;
        return 0;
    case SEEK_END:
        size = file->inode->size;
        file->offset = size + offset;
        return 0;
    default:
        return -1;
    }
}

int vfs_readdir (vfs_file_t *dir, unsigned index, char *name, unsigned maxlen) 
{
    (void)dir;
    (void)index;
    (void)name;
    (void)maxlen;
    return -ENOSYS;
}

//-------------------------------------------------------------------------------- getattr / setattr

int vfs_getattr (vfs_inode_t *inode, struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

int vfs_setattr (vfs_inode_t *inode, const struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
