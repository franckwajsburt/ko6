/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.c
  \author   Franck Wajsburt
  \brief    Virtual File System

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

\*------------------------------------------------------------------------------------------------*/

#ifndef _VFS_H_
#define _VFS_H_

#include <hal/devices/blockdev.h>

typedef struct vfs_inode_s {
    struct superblock_s *sb;        ///< Filesystem this inode belongs to
    unsigned index;                 ///< Inode identifile in the current superblock
    unsigned size;                  ///< File size in bytes
    unsigned short mode;            ///< File type and permissions 
    unsigned short refcount;        ///< Reference count 
    void *data;                     ///< physucak fs specific data (real inode)
} vfs_inode_t;

typedef struct superblock_s {
    blockdev_t *bdev;               ///< Block device on which this FS is mounted
    const struct fs_ops_s *ops;     ///< Phisical filesystem operation table
    struct vfs_inode_s *root;       ///< Virtual root inode of the mounted filesystem
    void *fs_data;                  ///< Private data (disk volume)
} superblock_t;

struct fs_ops_s {
    /**
     * \brief  Mount the a real filesystem on a given block device.
     *         This function reads the inode table (first block), sets up the superblock,
     *         and creates the synthetic root inode (index 0).
     * \param  sb   Pointer to the superblock to initialize.
     * \param  bdev Block device on which the filesystem is mounted.
     * \return 0 on success, -EIO or -ENOMEM on failure.
     * \note   fs1 : fs1_mount
     */
    int (*mount)(superblock_t *sb, blockdev_t *bdev);

    /**
     * \brief  Lookup a file or directory name in a parent directory.
     *         Given a directory inode and a file name, this function searches for a matching 
     *         entry and returns a new inode structure if found.
     * \param  sb   Pointer to the superblock associated with the filesystem.
     * \param  dir  Pointer to the parent directory inode 
     * \param  name Name of the file or directory to look up (null-terminated string).
     * \return A pointer to the corresponding vfs_inode_t if found, or NULL if not found.
     * \note   fs1 : fs1_lookup (only supports flat namespace, root directory only)
     */
    vfs_inode_t * (*lookup)(superblock_t *sb, vfs_inode_t *dir, const char *name);

    /**
     * \brief  Read data from a file stored in the real file system.
     * \param  ino    Pointer to the VFS inode representing the file.
     * \param  buffer Pointer to the user buffer to fill.
     * \param  offset Offset in bytes from the beginning of the file.
     * \param  size   Number of bytes to read.
     * \return The number of bytes actually read, or -EIO on I/O error.
     * \note   fs1 : fs1_read
     */
    int (*read)(vfs_inode_t *ino, void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Unmount the filesystem and release its resources.
     * \param  sb Pointer to the superblock to unmount.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*unmount)(superblock_t *sb);

    /**
     * \brief  Write data to a file stored in the filesystem.
     * \param  ino    Pointer to the VFS inode representing the file.
     * \param  buffer Pointer to the data to write.
     * \param  offset Offset in bytes from the beginning of the file.
     * \param  size   Number of bytes to write.
     * \return Number of bytes written, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*write)(vfs_inode_t *ino, const void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Create a new regular file in the directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the file to create.
     * \param  mode     Permissions and file type.
     * \param  new_ino  Output pointer to the newly created inode.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*create)(vfs_inode_t *dir, const char *name, mode_t mode, vfs_inode_t **new_ino);

    /**
     * \brief  Create a new directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the directory to create.
     * \param  mode     Permissions and file type.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*mkdir)(vfs_inode_t *dir, const char *name, mode_t mode);

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
     * \param  index  Index of the entry to retrieve (0-based).
     * \param  name   Output buffer for the entry name.
     * \param  maxlen Maximum number of characters to write into name.
     * \return 0 on success, or -ENOENT if index is out of bounds.
     * \note   fs1 : not implemented (NULL)
     */
    int (*readdir)(vfs_inode_t *dir, unsigned index, char *name, unsigned maxlen);

    /**
     * \brief  Retrieve file attributes (stat-like information).
     * \param  ino     Inode for the file or directory.
     * \param  statbuf Output structure to fill with metadata.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*getattr)(vfs_inode_t *ino, struct stat *statbuf);

    /**
     * \brief  Change file attributes (e.g., permissions).
     * \param  ino     Inode to modify.
     * \param  statbuf New metadata to apply.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : not implemented (NULL)
     */
    int (*setattr)(vfs_inode_t *ino, const struct stat *statbuf);
};

#endif//_VFS_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
