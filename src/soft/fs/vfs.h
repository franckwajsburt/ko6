/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
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

typedef struct superblock_s {
    blockdev_t *bdev;               ///< Block device on which this FS is mounted
    const struct fs_ops_s *ops;     ///< Phisical filesystem operation table
    struct vfs_inode_s *root;       ///< Virtual root inode of the mounted filesystem
    void *fs_data;                  ///< Private data (disk volume)
} superblock_t;

typedef struct vfs_inode_s {
    struct superblock_s *sb;        ///< Filesystem this inode belongs to
    unsigned index;                 ///< Inode identifile in the current superblock
    unsigned size;                  ///< File size in bytes
    unsigned short mode;            ///< File type and permissions 
    unsigned short refcount;        ///< Reference count 
    void *data;                     ///< physucak fs specific data (real inode)
} vfs_inode_t;

typedef struct vfs_file_s {
    struct vfs_inode_s *inode;      ///< vfs inode of the open file 
    unsigned offset;                ///< current read/write position
} vfs_file_t;

enum whence_e {
    SEEK_SET,                       ///< file offset is set to offset bytes
    SEEK_CUR,                       ///< file offset is set to current location plus offset bytes
    SEEK_END                        ///< file offset is set to the file size plus offset bytes
};

/** 
 * \brief real file system API
 *
 *  mount     Mount the a real filesystem on a given block device.
 *  lookup    Lookup a file or directory name in a parent directory.
 *  read      Read data from a file stored in the real file system.
 *  unmount   Unmount the filesystem and release its resources.
 *  write     Write data to a file stored in the filesystem.
 *  create    Create a new regular file in the directory.
 *  mkdir     Create a new directory.
 *  unlink    Remove a file or directory.
 *  readdir   Read the contents of a directory (enumerate entries).
 *  getattr   Retrieve file attributes (stat-like information).
 *  setattr   Change file attributes (e.g., permissions).
 */
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

/**
 * \brief Mount a filesystem on a block device.
 * \param sb   Pointer to an empty superblock (allocated by the caller).
 * \param bdev Block device where the filesystem is located.
 * \param ops  Filesystem operations (e.g., fs1_ops).
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_mount(superblock_t *sb, blockdev_t *bdev, struct fs_ops_s *ops);

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
 * \brief Move the current file offset.
 * \param file   Open file pointer.
 * \param offset Byte offset to move.
 * \param whence One of SEEK_SET, SEEK_CUR, or SEEK_END.
 * \return 0 on success, negative error code on failure.
 */
extern int vfs_seek (vfs_file_t *file, int offset, int whence);

/**
 * \brief close file
 * \return 0 on success
 */
extern int vfs_close (vfs_file_t *file);

#endif//_VFS_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
