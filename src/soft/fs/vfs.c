/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.c
  \author   Franck Wajsburt
  \brief    Virtual File System

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

//--------------------------------------------------------------------------------------------------
// General VFS initialisation
//--------------------------------------------------------------------------------------------------

errno_t vfs_init (void)
{
    errno_t err = vfs_filesystem_register (&fs1_ops);       // Register the filesystem type first
    if (err != SUCCESS && err != -EEXIST) return err;

    superblock_t *sb = vfs_superblock_alloc ();             // Create a new superblock
    if (!sb) return -ENOMEM;

    blockdev_t *bdev = blockdev_get (0);                    // default block dev 0 for partition '/'
    if (!bdev) { kfree (sb); return -ENODEV; }

    int ret = vfs_mount ("/", sb, bdev, &fs1_ops);          // root '/' fs --> create root inode
    if (ret != 0) { kfree (sb); return ret; }

    INFO ("vfs_init: '/' (fs1) successfully mounted on block device 0");
    return SUCCESS;
}

//--------------------------------------------------------------------------------------------------
// VFS API exposed to syscall layer
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------- file system API

#define VFS_FILESYSTEM_MAX 8                                ///< Max nb of supported filesystems

static const vfs_fs_type_t *fs_registry[VFS_FILESYSTEM_MAX];///< Static registry of filesystem types

const vfs_fs_type_t *vfs_filesystem_get (const char *name)
{
    if (!name) return NULL;                                 // nothing to search
    for (int i = 0; i < VFS_FILESYSTEM_MAX; ++i) {          // browse the registery table
        const vfs_fs_type_t *fs = fs_registry[i];           // get the registered fs
        if (fs && strcmp (fs->name, name) == 0)             // if there is a fs and name found
            return fs;                                      // return the fs 
    }
    return NULL;                                            // not found
}

errno_t vfs_filesystem_register (const vfs_fs_type_t *ops)
{
    if (!ops || !ops->name) return -EINVAL;                 // check argument validity
    if (vfs_filesystem_get (ops->name)) return -EEXIST;     // already registered
    for (int i = 0; i < VFS_FILESYSTEM_MAX; ++i) {          // Find a free slot
        if (fs_registry[i] == NULL) {                       // free slot found
            fs_registry[i] = ops;                           // register ops
            return SUCCESS;                                 // success
        }
    }
    return -ENOSPC;                                         // registry full
}

//---------------------------------------- mounted file system tables: definition & helper functions

#define VFS_MOUNT_MAX 15                                    ///< Max number of mounted filesystems

struct mount_point_s {
    char *path;                                             ///< absolut path where to mount: "/mnt"
    vfs_inode_t *inode;                                     ///< mount point inode in parent sb
    superblock_t *sb;                                       ///< mounted file system
} Vfs_mount_table [VFS_MOUNT_MAX];                          ///< global mount table
mnt_id_t Vfs_mount_max;                                     ///< actual mounted file system number

#define IS_VALID_MNT_ID(id) ((id)>=0 && (id)<Vfs_mount_max) ///< Check mount id validity

/// Iterate over all mount IDs in Vfs_mount_table[]
/// This macro abstracts away the implementation detail (array vs future dynamic structure)
#define vfs_mount_foreach_id(id)     for (mnt_id_t id = 0; id < Vfs_mount_max; ++id)
#define vfs_mount_foreach_id_all(id) for (mnt_id_t id = 0; id < VFS_FILESYSTEM_MAX; ++id)

/**
 * \brief Vfs_mount_table accessors which hide the internal structure of the Vfs_mount_table
 */
static char *vfs_mount_path_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].path : NULL;
}
static void vfs_mount_path_set (mnt_id_t id, char *path) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].path = path;
}
static vfs_inode_t *vfs_mount_inode_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].inode : NULL;
}
static void vfs_mount_inode_set (mnt_id_t id, vfs_inode_t *inode) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].inode = inode;
}
static superblock_t *vfs_mount_sb_get (mnt_id_t id) {
    return IS_VALID_MNT_ID(id) ? Vfs_mount_table[id].sb : NULL;
}
static void vfs_mount_sb_set (mnt_id_t id, superblock_t *sb) {
    if (IS_VALID_MNT_ID(id)) Vfs_mount_table[id].sb = sb;
}

/**
 * \brief Update the greater mnt_id used for Vfs_mount_table
 * \param id the mnt_id just modified
 * \return nothing but a possible change of the global variable Vfs_mount_max
 */
static void vfs_mount_update_max (mnt_id_t id) {
    if (id >= Vfs_mount_max)                                    // a new place -> max increments
        Vfs_mount_max = id+1;                                   // max is a number of places -> +1
    else if (id < Vfs_mount_max) {                              // if it a freeing
        mnt_id_t i = Vfs_mount_max-1;                           // begin search for the actual max
        while ((i>=0) && (Vfs_mount_table[i].path == NULL)) i--;// search until non-NULL or end
        Vfs_mount_max = i+1;                                    // new max found
    }
}

/**
 * \brief allocate a new entry into the Vfs_mount_table
 * \return a free mnt_id or -ENOSPC if no space left
 */
static errno_t vfs_mount_alloc (void)
{
    vfs_mount_foreach_id_all (id) {                         // browse the entire mount table
        if (vfs_mount_path_get (id) == NULL) {              // empty place found
            vfs_mount_update_max (id);                      // update the mounted fs max  
            return id;                                      // return the mnt_id in mount table
        }
    }
    return -ENOSPC;                                         // place or path not found
}

/**
 * \brief Registers a superblock mounted under a given path.
 * \param path  Path of the mount point (e.g., “/mnt”, “/”)
 * \param sb    Superblock to register
 * \return mnt_id in the table or -ENOSPC if failed
 */
static errno_t vfs_mount_register (const char *path, vfs_inode_t *inode, superblock_t *sb)
{
    mnt_id_t id = vfs_mount_alloc ();                       // get a place in mount table
    if (id < 0) return -ENOSPC;                             // no space left
    sb->mnt_id = id;                                        // sb identifier in the mount table
    vfs_mount_path_set (id, kstrdup(path));                 // register path 
    vfs_mount_inode_set(id, inode);                         // register mnt point inode in parent sb
    vfs_mount_sb_set   (id, sb);                            // register superblock
    return id;                                              // success
}

/**
 * \brief Unregisters a mounted superblock
 * \param path  Path of the mount point (e.g., “/mnt”, “/”)
 * \return the superblock previously mounterd or NULL if path not found
 */
static superblock_t *vfs_mount_unregister (const char *path)
{
    mnt_id_t id = vfs_mount_lookup (path);                  // retreive the place in mount table
    if (id < 0) return NULL;                                // path not found 
    superblock_t *sb = vfs_mount_sb_get (id);               // retreive the sb
    kfree (vfs_mount_path_get (id));                        // free path name
    vfs_mount_path_set (id, NULL);                          // unregister path 
    vfs_mount_inode_set (id, NULL);                         // unregister inode
    vfs_mount_sb_set (id, NULL);                            // unregister superblock
    vfs_mount_update_max (id);                              // update the max actual mounted fs
    sb->mnt_id = -1;                                        // superblock is unregistered
    return sb;                                              // ret superblock previously registered
}

//---------------------------------------------------------- superblock management / mount / unmount

superblock_t *vfs_superblock_alloc (void) 
{
    superblock_t *sb = kmalloc (sizeof(superblock_t));
    if (!sb) return NULL;               // no space left
    sb->mnt_id = -1;                    // unregistered by default
    return sb;
}

errno_t vfs_kern_mount (superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops)
{
    if (!sb || !bdev || !ops) return -EINVAL;               // check arguments validity
    sb->ops = ops;                                          // assign the filesystem operations
    sb->bdev = bdev;                                        // assign the block device
    sb->fs_data = NULL;                                     // will be init by fs-specific mount
    return ops->mount (sb, bdev);                           // call fs-specific mount function
}

errno_t vfs_kern_unmount (superblock_t *sb)
{
    if (!sb || !sb->ops || !sb->ops->mount) return -EINVAL; // check arguments validity
    return sb->ops->unmount (sb);                           // Call the fs-specific mount function
}

errno_t vfs_mount (const char *path, superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops)
{
    vfs_inode_t *inode = vfs_inode_resolve (path);          // retreive path's inode BEFORE mounting
    int ret = vfs_kern_mount (sb, bdev, ops);               // mount the new file system
    if (ret >= 0) {
        ret = vfs_mount_register (path, inode, sb);         // find a place in vfs_mount_table
        if (ret < 0)
            vfs_kern_unmount (sb);                          // rollback mount if registration failed
    }
    return ret;                                             // return < 0 on fealure
}

errno_t vfs_umount (const char *path)
{
    superblock_t *sb = vfs_mount_unregister (path);         // unregister from the Vfs_mount_table
    return vfs_kern_unmount (sb);                           // Call the fs-specific mount function
}

mnt_id_t vfs_mount_lookup (const char *path)
{
    vfs_mount_foreach_id (id) {                             // browse the mount table
        char *curpath = vfs_mount_path_get (id);            // get the current path
        if (path && (strcmp (path, curpath) == 0))          // path found
            return id;                                      // return the mnt_id in mount table
    }
    return -ENOENT;                                         // place or path not found
}

superblock_t *vfs_mount_resolve (const char *path)
{
    superblock_t *best_sb = NULL;                           // will the the best superblock
    int best_len = -1;                                      // the one with the greater path length
    if (!path || path[0] != '/') return NULL;               // path has to be absolute
    vfs_mount_foreach_id (id) {                             // browse the mount table
        const char *mnt = vfs_mount_path_get (id);          // get the current mount path
        superblock_t *sb = vfs_mount_sb_get (id);           // and its superblock
        if (!mnt || !sb) continue;                          // if nothing registered continue
        int len = strlen (mnt);                             // get the mount path length
        if (len > best_len &&                               // the current length is greater
            strncmp (path, mnt, len) == 0 &&                // and the path begin with mnt
            (path[len] == '/' || path[len] == '\0')) {      // and the path is not badly cut
            best_sb = sb;                                   // then sb is yhe new best choice
            best_len = len;                                 // for that length
        }
    }
    return best_sb;                                         // return the best superblock
}

//---------------------------------------------------------------------------- lookup / open / close

vfs_inode_t *vfs_resolve (vfs_inode_t *dir, const char *path)
{
    if (!path || path[0] == '\0') return NULL;              // nothing to resolve

    superblock_t *sb;
    vfs_inode_t *inode;

    char *path_copy = kmalloc(PAGE_SIZE);                   // allocate temporary buffer
    strncpy (path_copy, path, PAGE_SIZE);                   // duplicate path
    path_copy[PAGE_SIZE - 1] = '\0';                        // ensure null-termination

    char *parts[32];                                        // max 32 path segments
    int count = strsplit(path_copy, "/", parts, 32);        // split path into components

    if (path[0] == '/') {
        if (dir != NULL) goto resolve_fail;                 // inconsistent: absolute path + dir
        sb = vfs_mount_resolve(path);                       // find the matching superblock
        if (!sb || !sb->root) goto resolve_fail;            // no root found
        inode = sb->root;
    } else {
        if (dir == NULL) goto resolve_fail;                 // relative path needs a base inode
        sb = dir->sb;
        inode = dir;
    }

    for (int i = 0; i < count; ++i) {
        if (parts[i][0] == '\0' || strcmp(parts[i], ".") == 0) continue;  // skip empty or "."

        inode = sb->ops->lookup(sb, inode, parts[i]);       // lookup next component in real FS
        if (!inode) goto resolve_fail;

        vfs_mount_foreach_id(id) {                          // Check if this inode is a mount point
            superblock_t *mnt_sb = vfs_mount_sb_get(id);    // retrieve superblock from mount table
            if (mnt_sb && vfs_mount_inode_get(id) == inode){// match found: inode is a mount root
                sb = mnt_sb;                                // switch to mounted filesystem
                inode = sb->root;                           // reset to its root
                break;
            }
        }
    }

    kfree(path_copy);
    return inode;

resolve_fail:
    kfree(path_copy);
    return NULL;
}

vfs_file_t *vfs_open (vfs_inode_t *dir, const char *path)
{
    vfs_inode_t *inode = vfs_resolve (dir, path);           // retrieve the path's inode 
    if (!inode) return NULL;
    vfs_file_t *file = kmalloc (sizeof(vfs_file_t));        // allocate a new file 
    if (!file) { vfs_inode_release (inode); return NULL; }  
    vfs_inode_get (inode);                                  // increase refcount because of the file
    file->inode = inode;                                    // iniatialize file
    file->offset = 0;                                       // start file access from the beginning
    return file;                                            // at last, return the new file
}

errno_t vfs_close (vfs_file_t *file)
{
    vfs_inode_release (file->inode);
    kfree (file);
    return SUCCESS;
}

//-------------------------------------------------------------------- read / write / seek / readdir

errno_t vfs_read (vfs_file_t *file, void *buffer, size_t size)
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

errno_t vfs_write (vfs_file_t *file, const void *buffer, size_t size)
{
    (void)file;
    (void)buffer;
    (void)size;
    return -ENOSYS;
}

errno_t vfs_seek (vfs_file_t *file, int offset, int whence)
{
    unsigned size;
    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        return SUCCESS;
    case SEEK_CUR:
        file->offset += offset;
        return SUCCESS;
    case SEEK_END:
        size = file->inode->size;
        file->offset = size + offset;
        return SUCCESS;
    default:
        return FAILURE;
    }
}

errno_t vfs_readdir (vfs_file_t *dir, unsigned index, char *name, unsigned maxlen)
{
    (void)dir;
    (void)index;
    (void)name;
    (void)maxlen;
    return -ENOSYS;
}

//-------------------------------------------------------------------------------- getattr / setattr

errno_t vfs_getattr (vfs_inode_t *inode, struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

errno_t vfs_setattr (vfs_inode_t *inode, const struct stat *statbuf)
{
    (void)inode;
    (void)statbuf;
    return -ENOSYS;
}

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

vfs_inode_t *vfs_inode_create (superblock_t *sb, ino_t ino, size_t size, mode_t mode, void *data)
{
    vfs_inode_t *inode = kmalloc (sizeof (vfs_inode_t));     // allocate a new vfs_inode
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

vfs_inode_t *vfs_inode_resolve (const char *path)
{
    superblock_t *sb = vfs_mount_resolve(path);             // Find the sb matching the path prefix
    if (!sb) return NULL;                                   // No mount point found for this path
    const char *subpath;                                    // remaining path without point prefix
    subpath = path + strlen(vfs_mount_path_get(sb->mnt_id));// Remove mount point prefix
    if (*subpath == '/') subpath++;                         // Skip leading slash if present
    return vfs_resolve (sb->root, subpath);                 // resolve the remaining relative path
}

void vfs_inode_get (vfs_inode_t *inode)
{
    inode->refcount++;                                      // there is another reference
}

void vfs_inode_release (vfs_inode_t *inode)
{
    if (--(inode->refcount) == 0) {                         // decrement ref nb, if 0 then
        vfs_inode_cache_remove (inode);                     // remode the inode from cache
        kfree (inode);                                      // and the inode itself
    }
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
