/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-28
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.c
  \author   Franck Wajsburt
  \brief    VFS - Virtual File System layer

  Responsibilities:
   - Manage file opening, reading, seeking and closing
   - Manage inode lifetime via reference counting
   - Provide a clean abstraction between filesystems and user programs
 
  Each element isolates a level of abstraction
  Level       Role
  ------------------------------------------------------------------------------------------
  vfs_inode   Information about a file or directory
              inode: generic, abstracted by the VFS, with no direct knowledge of the device
  superblock  Information about a mounted volume
              superblock: represents the entire file system mounted on a block device
  blockdev    Physical interface to the device
              blockdev: represents physical access to storage (via block driver)
  minor       Physical device number (type + instance)
              minor: identifies the hardware device (storage, partition, etc.)

  There are no shortcuts like embedding a superblock pointer directly into the file structure. 
  This is intentional, each abstraction has its own role:
  - A file represents an open file instance.
  - An inode represents the file metadata.
  - A superblock represents the mounted filesystem.

  Respecting these separations keeps the architecture clean, understandable, and easy to evolve.
  Shortcuts may bring minor speed gains, but they destroy the clarity and independence of layers, 
  making future maintenance harder. In this system, clarity, correctness, and long-term 
  maintainability are prioritized over premature optimizations.

\*------------------------------------------------------------------------------------------------*/

#ifndef _VFS_H_
#define _VFS_H_

#include <hal/devices/blockdev.h>
#include <common/vfs_stat.h>

//--------------------------------------------------------------------------------------------------
// VFS main API
//--------------------------------------------------------------------------------------------------

typedef struct superblock_s {
    blockdev_t *bdev;               ///< Block device on which this FS is mounted
    const struct fs_ops_s *ops;     ///< Phisical filesystem operation table
    struct vfs_inode_s *root;       ///< Virtual root inode of the mounted filesystem
    void *fs_data;                  ///< Private data (disk volume)
} superblock_t;

/**
 * \note Each vfs_inode_t starts with refcount = 1 when created.
 * vfs_inode_get() must be called to hold an additional reference for active usage.
 * vfs_inode_release() decrements the refcount, freeing the inode when it reaches zero.
 */
typedef struct vfs_inode_s {
    struct superblock_s *sb;        ///< Filesystem this inode belongs to
    ino_t    ino;                   ///< Inode identifier in the current superblock
    size_t   size;                  ///< File size in bytes
    unsigned short mode;            ///< File type and permissions 
    unsigned short refcount;        ///< Reference count 
    void *data;                     ///< Real fs specific data (real inode pointer)
} vfs_inode_t;

typedef struct vfs_file_s {
    struct vfs_inode_s *inode;      ///< vfs inode of the open file 
    unsigned offset;                ///< current read/write position
} vfs_file_t;

/** 
 * \brief struct fs_ops_s : Virtual File System API to implement with real file system function
 *    mount     Mount the a real filesystem on a given block device.
 *    unmount   Unmount the filesystem and release its resources.
 *    lookup    Lookup a file or directory name in a parent directory.
 *    read      Read data from a file stored in the real file system.
 *    write     Write data to a file stored in the filesystem.
 *    create    Create a new regular file in the directory.
 *    mkdir     Create a new directory.
 *    unlink    Remove a file or directory.
 *    readdir   Read the contents of a directory (enumerate entries).
 *    getattr   Retrieve file attributes (stat-like information).
 *    setattr   Change file attributes (e.g., permissions).
 */
struct fs_ops_s {
    /**
     * \brief  Mount the a real filesystem on a given block device.
     *         This function reads the inode table (first block), sets up the superblock,
     *         and creates the synthetic root inode (inode index)=ino=0.
     * \param  sb   Pointer to the superblock to initialize.
     * \param  bdev Block device on which the filesystem is mounted.
     * \return 0 on success, -EIO or -ENOMEM on failure.
     * \note   fs1 : fs1_mount
     */
    int (*mount)(superblock_t *sb, blockdev_t *bdev);

    /**
     * \brief  Unmount the filesystem and release its resources.
     * \param  sb Pointer to the superblock to unmount.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*unmount)(superblock_t *sb);

    /**
     * \brief  Lookup a file or directory name in a parent directory.
     *         Given a directory inode and a file name, this function searches for a matching 
     *         entry and returns a new inode structure if found.
     * \param sb    Pointer to the superblock where the file is located.
     * \param dir   Parent directory inode (ignored for fs1, single directory filesystem).
     * \param name  File name to search for (no slashes allowed).
     * \return Pointer to the vfs_inode_t if the file is found, NULL otherwise.
     * \note The 'name' must be a simple file name without any '/' character.
     *       The fs1 filesystem does not support hierarchical directories: all files 
     *       are located at the root.
     */
    vfs_inode_t * (*lookup)(superblock_t *sb, vfs_inode_t *dir, const char *name);

    /**
     * \brief  Read data from a file stored in the real file system.
     * \param  inode  Pointer to the VFS inode representing the file.
     * \param  buffer Pointer to the user buffer to fill.
     * \param  offset Offset in bytes from the beginning of the file.
     * \param  size   Number of bytes to read.
     * \return The number of bytes actually read, or -EIO on I/O error.
     * \note   fs1 : fs1_read
     */
    int (*read)(vfs_inode_t *inode, void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Write data to a file stored in the filesystem.
     * \param  inode  Pointer to the VFS inode representing the file.
     * \param  buffer Pointer to the data to write.
     * \param  offset Offset in bytes from the beginning of the file.
     * \param  size   Number of bytes to write.
     * \return Number of bytes written, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*write)(vfs_inode_t *inode, const void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Create a new regular file in the directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the file to create.
     * \param  mode     Permissions and file type.
     * \return the newly created inode or NULL if impossible
     * \note   fs1 : not implemented (NULL)
     */
    vfs_inode_t * (*create)(vfs_inode_t *dir, const char *name, mode_t mode);

    /**
     * \brief  Create a new directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the directory to create.
     * \param  mode     Permissions and file type.
     * \return Pointer to the vfs_inode_t on new dir, NULL otherwise.
     * \note   fs1 : not implemented (NULL)
     */
    vfs_inode_t * (*mkdir)(vfs_inode_t *dir, const char *name, mode_t mode);

    /**
     * \brief  Remove a file or directory.
     * \param  dir  Parent directory inode.
     * \param  name Name of the file or directory to remove.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*unlink)(vfs_inode_t *dir, const char *name);

    /**
     * \brief  Read the contents of a directory (enumerate entries).
     * \param  dir    Directory inode.
     * \param  ino    Index of the entry to retrieve (0-based).
     * \param  name   Output buffer for the entry name.
     * \param  maxlen Maximum number of characters to write into name.
     * \return 0 on success, or -ENOENT if ino is out of bounds.
     * \note   fs1 : not implemented (NULL)
     */
    int (*readdir)(vfs_inode_t *dir, ino_t ino, char *name, unsigned maxlen);

    /**
     * \brief  Retrieve file attributes (stat-like information).
     * \param  inode   Inode for the file or directory.
     * \param  statbuf Output structure to fill with metadata.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*getattr)(vfs_inode_t *inode, struct stat *statbuf);

    /**
     * \brief  Change file attributes (e.g., permissions).
     * \param  inode   Inode to modify.
     * \param  statbuf New metadata to apply.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*setattr)(vfs_inode_t *inode, const struct stat *statbuf);
};

//--------------------------------------------------------------------------------------------------
// VFS API
//--------------------------------------------------------------------------------------------------

/**
 * \brief Mount a filesystem on a block device.
 * \param sb   Pointer to an empty superblock (allocated by the caller).
 * \param bdev Block device where the filesystem is located.
 * \param ops  Filesystem operations (e.g., fs1_ops).
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_mount(superblock_t *sb, blockdev_t *bdev, struct fs_ops_s *ops);

/**
 * \brief Unmount a filesystem from its block device.
 * \param sb Pointer to the superblock to unmount.
 * \return 0 on success, or a negative error code.
 */
extern int vfs_umount(superblock_t *sb);

/**
 * \brief Lookup a file by path in a mounted filesystem.
 * \param sb   Pointer to the superblock where the file is located.
 * \param dir  Inode where the path is seached
 * \param path Path of the file (relative to the root, no slashes for now).
 * \return Pointer to a vfs_inode_t if the file is found, NULL otherwise.
 * \note If the path starts with '/', the lookup starts from the root directory (sb->root),
 *       and the 'dir' parameter is ignored. Otherwise, lookup starts from the given 'dir'.
 */
extern vfs_inode_t *vfs_lookup(superblock_t *sb, vfs_inode_t *dir, const char *path);

/**
 * \brief Open a file from its path.
 * \param sb    Pointer to the superblock where the file is located.
 * \param path  Path of the file (relative to root, no slashes for now).
 * \return Pointer to an allocated vfs_file_t, or NULL on error.
 */
extern vfs_file_t *vfs_open (superblock_t *sb, const char *path);

/**
 * \brief Read data from an open file.
 * \param file   Open file pointer.
 * \param buffer Destination buffer.
 * \param size   Number of bytes to read.
 * \return Number of bytes read, or negative error code on failure.
 */
extern int vfs_read (vfs_file_t *file, void *buffer, unsigned size);


/**
 * \brief Write data to an open file.
 * \param file   Open file pointer.
 * \param buffer Source buffer.
 * \param size   Number of bytes to write.
 * \return Number of bytes written, or negative error code on failure.
 */
extern int vfs_write(vfs_file_t *file, const void *buffer, unsigned size);

enum whence_e {
    SEEK_SET,  ///< file offset is set to offset bytes
    SEEK_CUR,  ///< file offset is set to current location plus offset bytes
    SEEK_END   ///< file offset is set to the file size plus offset bytes
};

/**
 * \brief Move the current file offset.
 * \param file   Open file pointer.
 * \param offset Byte offset to move.
 * \param whence One of SEEK_SET, SEEK_CUR, or SEEK_END.
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_seek (vfs_file_t *file, int offset, int whence);

/**
 * \brief Read a directory entry.
 * \param file   Open directory file pointer.
 * \param index  Index of the entry.
 * \param name   Buffer to store the entry name.
 * \param maxlen Size of the name buffer.
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_readdir(vfs_file_t *file, unsigned index, char *name, unsigned maxlen);

/**
 * \brief Get attributes of an inode.
 * \param inode  Pointer to the inode.
 * \param statbuf Output buffer for the attributes.
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_getattr(vfs_inode_t *inode, struct stat *statbuf);

/**
 * \brief Set attributes of an inode.
 * \param inode  Pointer to the inode.
 * \param statbuf New attributes to apply.
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_setattr(vfs_inode_t *inode, const struct stat *statbuf);

/**
 * \brief close file
 * \return 0 on success
 */
extern int vfs_close (vfs_file_t *file);

//--------------------------------------------------------------------------------------------------
// inode cache API
//--------------------------------------------------------------------------------------------------

/**
 * \brief Lookup an inode in the VFS inode cache.
 * \param sb  Pointer to the superblock where the inode should belong.
 * \param ino Index of the inode to search for.
 * \return Pointer to the vfs_inode_t if found, NULL otherwise.
 */
extern vfs_inode_t *vfs_inode_lookup(superblock_t *sb, ino_t ino);

/**
 * \brief Increment the reference count of an inode.
 * \param inode Pointer to the inode whose reference count is incremented.
 */
extern void vfs_inode_get(vfs_inode_t *inode);

/**
 * \brief Decrement the reference count of an inode and free it if it reaches zero.
 * \param inode Pointer to the inode whose reference count is decremented.
 */
extern void vfs_inode_release(vfs_inode_t *inode);

/**
 * \brief Insert an inode into the VFS inode cache.
 * \param inode Pointer to the vfs_inode_t to insert.
 * \note For now, this function does nothing because there is no inode cache yet.
 */
extern void vfs_inode_cache_insert(vfs_inode_t *inode);

/**
 * \brief Remove an inode from the VFS inode cache.
 * \param inode Pointer to the vfs_inode_t to remove.
 * \note For now, this function does nothing because there is no inode cache yet.
 */
extern void vfs_inode_cache_remove(vfs_inode_t *inode);

#endif//_VFS_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
