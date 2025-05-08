/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-02
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/vfs.h
  \author   Franck Wajsburt
  \brief    Core structures and API for the Virtual File System (VFS)

  This file defines the generic interface between user-level operations and real filesystems. 
  Each real filesystem must provide its own implementation through the vfs_fs_type_t structure.

  The VFS separates filesystem concepts into three main categories:
   - Superblocks (`superblock_t`): Represent mounted volumes (filesystems).
   - Dentries and Inodes (`vfs_dentry_t`, `vfs_inode_t`): Represent file and directory metadata.
   - Files (`vfs_file_t`, `vfs_dirent_t`): Represent active file descriptors opened by processes.
  
  VFS Responsibilities:
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
  Shortcuts may bring small speed gains, but they destroy the clarity and independence of layers, 
  making future maintenance harder. In this system, clarity, correctness, and long-term 
  maintainability are prioritized over premature optimizations.

\*------------------------------------------------------------------------------------------------*/

#ifndef _VFS_H_
#define _VFS_H_

#include <hal/devices/blockdev.h>
#include <common/vfs_stat.h>

//--------------------------------------------------------------------------------------------------
// General VFS initialisation
//--------------------------------------------------------------------------------------------------

errno_t vfs_init (void);
void vfs_test (void);

//--------------------------------------------------------------------------------------------------
// VFS main API : Types & Variables
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------- file system API

/**
 * \brief : Virtual File System API to implement with real file system function (see bellow)
 */
typedef struct vfs_fs_type_s vfs_fs_type_t;

//------------------------------------------------------------------------ superblock / mount points

/**
 * \brief Represents a mounted filesystem.
 *        A superblock describes a filesystem instance mounted on a block device,
 *        and contains the operations and private data associated with that filesystem.
 */
typedef struct superblock_s {
    blockdev_t          *bdev;          ///< Block device on which this FS is mounted
    mnt_id_t             mnt_id;        ///< superblock identifier 
    const vfs_fs_type_t *ops;           ///< Phisical filesystem operation table (defined bellow)
    struct vfs_inode_s  *root;          ///< Virtual root inode of the mounted filesystem
    void                *fs_data;       ///< Private data (disk volume)
} superblock_t;

//----------------------------------------------------------------------------------- dentry / inode

/**
 * \brief VFS Directory Entry (dentry) structure.
 *        Associates a name (component of a path) to a vfs_inode_t.
 */
typedef struct vfs_dentry_s {
    struct vfs_inode_s  *inode;         ///< Associated inode
    struct vfs_dentry_s *parent;        ///< Parent dentry
    char name[];                        ///< Flexible array member for the entry name
} vfs_dentry_t;

/**
 * \brief VFS inode flags
 */
#define VFS_INODE_DIRTY   0x01          ///< Modified inode, should be written back to the fs
#define VFS_INODE_PINNED  0x02          ///< Must stay in cache
#define VFS_INODE_DELETED 0x04          ///< Unlinked but still used (will be freed when refcount=0)
#define VFS_INODE_LOCKED  0x08          ///< Temporarily locked (e.g. for update or synchronization)

/**
 * \brief Represents a file or directory in the VFS.
 *        A VFS inode stores metadata about a file or directory,
 *        such as its size, type, permissions, and a pointer to the real
 *        filesystem-specific inode structure.
 * \note  Each vfs_inode_t starts with refcount = 1 when created.
 *        vfs_inode_get () must be called to hold an additional reference for active usage.
 *        vfs_inode_release () decrements the refcount, freeing the inode when it reaches zero.
 */
typedef struct vfs_inode_s {
    superblock_t *sb;                   ///< Filesystem this inode belongs to
    ino_t    ino;                       ///< Inode identifier in the current superblock
    size_t   size;                      ///< File size in bytes
    mode_t   mode;                      ///< File type and permissions (S_IFDIR, S_IFREG, etc.)
    size_t   refcount;                  ///< Reference count 
    unsigned flags;                     ///< Internal VFS state flags (e.g. dirty, pinned)
    void     *data;                     ///< Real fs specific data (real inode pointer)
    struct   vfs_mapping_s *mapping;    ///< Data mapping for files or directory contents
    struct   vfs_dentry_set_s *dentries;///< Set of child dentries (used only for directories)
    list_t   list;                      ///< used to chain inodes
} vfs_inode_t;

//--------------------------------------------------------------------- opened files and directories

/**
 * \brief Represents an open file in the VFS.
 *        A VFS file structure tracks the state of an open file or directory descriptor,
 *        including the associated inode and the current read/write offset.
 */
typedef struct vfs_file_s {
    vfs_inode_t *inode;                 ///< vfs inode of the open file 
    unsigned offset;                    ///< current offset read/write position in file or directory
    void *data;                         ///< Optional opaque pointer for FS-specific state
} vfs_file_t;

/**
 * \brief Represents a single entry in a directory listing.
 *        This structure describes a file or subdirectory inside a directory,
 *        as returned by functions like vfs_readdir(). It includes a global identifier
 *        (via mount ID and inode number), file type and permission flags, and the entry name.
 *        It uses a flexible array member (`name[]`) to store the null-terminated name,
 *        allowing dynamic allocation depending on the actual name length.
 */
typedef struct vfs_dirent_s {
    mnt_id_t mnt_id;                    ///< ID of the mounted filesystem, <mnt_id,ino>==identifier
    ino_t ino;                          ///< Inode number within the filesystem
    mode_t mode;                        ///< File type and permissions (see vfs_stat.h)
    char name[];                        ///< Flexible array member for the file or directory name
} vfs_dirent_t;

//--------------------------------------------------------------------------------------------------
// VFS API exposed to the other kernel module (syscall, ...)
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------- file system API

/**
 * \brief Registers a filesystem type in the global VFS registry.
 *        This function adds a new filesystem implementation (e.g., fs1, tmpfs)
 *        to the internal registry so that it can later be mounted by name.
 *        It is typically called once at system initialization for each supported filesystem.
 * \param ops Pointer to the filesystem type descriptor (vfs_fs_type_t).
 *            Must not be NULL and must provide a valid `.name` field.
 * \return 0 on success,  -EINVAL (Invalid argument), *-ENOSPC  (Registry is full)
 */
errno_t vfs_filesystem_register(const vfs_fs_type_t *ops);

/**
 * \brief Retrieves a filesystem type by its name.
 *        This function looks up the VFS registry and returns the matching
 *        vfs_fs_type_t structure based on its string name (e.g., "fs1", "tmpfs").
 * \param name Name of the filesystem type to retrieve (e.g., "fs1").
 * \return Pointer to the registered filesystem type, or NULL if not found.
 */
const vfs_fs_type_t *vfs_filesystem_get (const char *name);

//---------------------------------------------------------- superblock management / mount / unmount

/**
 * \brief Allocate an empty and unregistered superblock
 */
superblock_t *vfs_superblock_alloc (void); 

/**
 * \brief Mount & register a filesystem on a block device.
 * \param path  Absolute path name where the new file system has to be mounted
 * \param sb    Pointer to an empty superblock (allocated by the caller).
 * \param bdev  Block device where the filesystem is located.
 * \param ops   Pointer to the filesystem operations structure (fs1_ops, etc).
 * \return 0 on success, negative error code on failure.
 * \note vfs_kern_mount() assigns 'ops' to 'sb->ops' before calling sb->ops->mount().
 */
errno_t vfs_mount (const char *path, superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops);

/**
 * \brief Unmount a filesystem from its block device.
 * \param path  path name of the file system to unmount.
 * \return 0 on success, or a negative error code.
 */
errno_t vfs_umount (const char *path);

/**
 * \brief mount a filesystem on a block device without register it
 * \param sb   Pointer to an empty superblock (allocated by the caller).
 * \param bdev Block device where the filesystem is located.
 * \param ops  Pointer to the filesystem operations structure (fs1_ops, etc).
 * \return 0 on success, negative error code on failure.
 * \note vfs_kern_mount() assigns 'ops' to 'sb->ops' before calling sb->ops->mount().
 */
errno_t vfs_kern_mount (superblock_t *sb, blockdev_t *bdev, const vfs_fs_type_t *ops);
  
/**
 * \brief unmount an unregistered filesystem 
 * \param sb   Pointer to superblock to unmount
 * \return 0 on success, negative error code on failure.
 */
errno_t vfs_kern_unmount (superblock_t *sb);

/**
 * \brief Finds the mount identifier of a registered fs
 * \param path Path of the mount point (must match exactly).
 * \return Pointer to the superblock or NULL if not found.
 */
mnt_id_t vfs_mount_lookup (const char *path);

/**
 * \brief Resolve the best mount point matching a given absolute path.
 * \param path Absolute path to resolve (must start with '/').
 * \return Pointer to the corresponding superblock, or NULL if no match.
 */
superblock_t *vfs_mount_resolve (const char *path);

//---------------------------------------------------------------------------- lookup / open / close

/**
 * \brief Resolve a relative path within a filesystem, starting from a directory inode.
 *        This function splits the path into components and resolves each of them
 *        using the filesystem's `lookup()` operation.
 * \param dir  The starting directory inode.
 * \param path A relative path (e.g., "foo/bar/baz").
 * \return Pointer to the resolved inode, or NULL on failure.
 */
vfs_inode_t *vfs_resolve (vfs_inode_t *dir, const char *path);

/**
 * \brief Opens a file by resolving its path, starting from a given directory inode.
 * \param dir  NULL for absolute path, or starting inode for relative path
 * \param path The path to resolve and open
 * \return Pointer to a file descriptor structure, or NULL on failure.
 */
vfs_file_t *vfs_open (vfs_inode_t *dir, const char *path);

/**
 * \brief close file
 * \return 0 on success
 */
errno_t vfs_close (vfs_file_t *file);

/**
 * \brief Opens a directory stream for reading entries.
 * \param path Absolute or relative path to the directory.
 * \return A pointer to a newly allocated vfs_file_t, or NULL on failure.
 */
vfs_file_t *vfs_opendir (vfs_inode_t *base, const char *path);

/**
* \brief Closes a directory stream and releases associated resources.
* \param dir Pointer to the open directory stream to close.
*/
void vfs_closedir (vfs_file_t *dir);

//-------------------------------------------------------------------- read / write / seek / readdir

/**
* \brief Read data from an open file.
* \param file   Open file pointer.
* \param buffer Destination buffer.
* \param size   Number of bytes to read.
* \return Number of bytes read, or negative error code on failure.
*/
errno_t vfs_read (vfs_file_t *file, void *buffer, unsigned size);

/**
* \brief Write data to an open file.
* \param file   Open file pointer.
* \param buffer Source buffer.
* \param size   Number of bytes to write.
* \return Number of bytes written, or negative error code on failure.
*/
errno_t vfs_write (vfs_file_t *file, const void *buffer, unsigned size);

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
errno_t vfs_seek (vfs_file_t *file, int offset, int whence);

/**
* \brief Reads the next directory entry in an open directory stream.
*        This function returns a dynamically allocated vfs_dirent_t describing the next
*        file or directory in the directory opened with vfs_opendir().
* \param dir Pointer to an open directory stream (vfs_file_t)
* \return A newly allocated vfs_dirent_t on success, or NULL when no more entries.
*         The caller must free the returned structure with kfree().
*/
vfs_dirent_t *vfs_readdir (vfs_file_t *dir);

//-------------------------------------------------------------------------------- getattr / setattr

/**
* \brief Get attributes of an inode.
* \param inode  Pointer to the inode.
* \param statbuf Output buffer for the attributes.
* \return 0 on success, negative error code on failure.
*/
errno_t vfs_getattr (vfs_inode_t *inode, struct stat *statbuf);

/**
* \brief Set attributes of an inode.
* \param inode  Pointer to the inode.
* \param statbuf New attributes to apply.
* \return 0 on success, negative error code on failure.
*/
errno_t vfs_setattr (vfs_inode_t *inode, const struct stat *statbuf);

//--------------------------------------------------------------------------------------------------
// VFS main API : Virtual File System API to implement with real file system function
//--------------------------------------------------------------------------------------------------

/** 
 *  mount     Mount a real filesystem on a given block device.
 *  unmount   Unmount the filesystem and release its resources.
 *  lookup    Lookup a file or directory name in a parent directory.
 *  read      Read data from a file stored in the real file system.
 *  write     Write data to a file stored in the filesystem.
 *  create    Create a new regular file in the directory.
 *  mkdir     Create a new directory.
 *  evict     Clean the fs-specific data from the inode
 *  unlink    Remove a file or directory.
 *  readdir   Read the contents of a directory (enumerate entries).
 *  getattr   Retrieve file attributes (stat-like information).
 *  setattr   Change file attributes (e.g., permissions).
 */
struct vfs_fs_type_s {

    /**
     * \brief  real file system name, such as "fs1"
     */
    const char *name;

    /**
     * \brief  Mount the a real filesystem on a given block device.
     *         This function reads the real inode table (first block), sets up the superblock,
     *         and creates the vfs root inode for the real fs root inode
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
     * \note   fs1 : fs1_unmount
     */
    errno_t (*unmount)(superblock_t *sb);

    /**
     * \brief  Lookup a file or directory name in a parent directory.
     *         Given a directory inode and a file name, this function searches for a matching 
     *         entry and returns a new inode structure if found.
     * \param  sb    Pointer to the superblock where the file is located.
     * \param  dir   Parent directory inode (ignored for fs1, single directory filesystem).
     * \param  name  File name to search for (no slashes allowed).
     * \return Pointer to the vfs_inode_t if the file is found, NULL otherwise.
     * \note   The 'name' must be a simple file name without any '/' character.
     *         The fs1 filesystem does not support hierarchical directories: all files 
     *         are located at the root.
     * \note   fs1 : fs1_lookup
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
    errno_t (*read)(vfs_inode_t *inode, void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Write data to a file stored in the filesystem.
     * \param  inode  Pointer to the VFS inode representing the file.
     * \param  buffer Pointer to the data to write.
     * \param  offset Offset in bytes from the beginning of the file.
     * \param  size   Number of bytes to write.
     * \return Number of bytes written, or a negative error code.
     * \note   fs1 : fs1_write
     */
    errno_t (*write)(vfs_inode_t *inode, const void *buffer, unsigned offset, unsigned size);

    /**
     * \brief  Create a new regular file in the directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the file to create.
     * \param  mode     Permissions and file type.
     * \return the newly created inode or NULL if impossible
     * \note   fs1 : fs1_create
     */
    vfs_inode_t * (*create)(vfs_inode_t *dir, const char *name, mode_t mode);

    /**
     * \brief  Create a new directory.
     * \param  dir      Parent directory inode.
     * \param  name     Name of the directory to create.
     * \param  mode     Permissions and file type.
     * \return Pointer to the vfs_inode_t on new dir, NULL otherwise.
     * \note   fs1 : fs1_mkdir
     */
    vfs_inode_t * (*mkdir)(vfs_inode_t *dir, const char *name, mode_t mode);

    /**
     * \brief  Clean the fs-specific data from the inode
     * \param  inode inode to clean.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : fs1_evict
     */
    errno_t (*evict)(vfs_inode_t *inode);

    /**
     * \brief  Remove a file or directory.
     * \param  dir  Parent directory inode.
     * \param  name Name of the file or directory to remove.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : fs1_unlink
     */
    errno_t (*unlink)(vfs_inode_t *dir, const char *name);

    /**
     * \brief  Read data from a directory stored in the real file system.
     * \param  inode  Pointer to the VFS inode representing the file.
     * \param  dirent Pointer to the buffer to fill with the diectory entry
     * \param  offset Offset in item from the beginning of the directory.
     * \return The number of entry read (0 or 1) or < 0 on I/O error.
     * \note   fs1 : fs1_readdir
     */
    errno_t (*readdir)(vfs_inode_t *dir, vfs_dirent_t *ent, size_t offset);

    /**
     * \brief  Retrieve file attributes (stat-like information).
     * \param  inode   Inode for the file or directory.
     * \param  statbuf Output structure to fill with metadata.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : fs1_getattr
     */
    errno_t (*getattr)(vfs_inode_t *inode, struct stat *statbuf);

    /**
     * \brief  Change file attributes (e.g., permissions).
     * \param  inode   Inode to modify.
     * \param  statbuf New metadata to apply.
     * \return 0 on success, or a negative error code.
     * \note   fs1 : fs1_setattr
     */
    errno_t (*setattr)(vfs_inode_t *inode, const struct stat *statbuf);
};

//--------------------------------------------------------------------------------------------------
// dentry API
//--------------------------------------------------------------------------------------------------

/**
 * \brief Lookup a dentry by name in a directory.
 * \param parent Parent directory dentry.
 * \param name   Name of the file or directory.
 * \return Pointer to the vfs_dentry_t if found, NULL otherwise.
 */
vfs_dentry_t *vfs_dentry_lookup (vfs_dentry_t *parent, const char *name);

/**
 * \brief Create a new dentry under a parent directory.
 * \param parent Parent directory dentry.
 * \param name   Nam:e of the new entry.
 * \param inode  Associated inode.
 * \return Pointer to the newly created vfs_dentry_t.
 */
vfs_dentry_t *vfs_dentry_create (vfs_dentry_t *parent, const char *name, vfs_inode_t *inode);

/**
 * \brief Destroy a dentry and free its resources.
 * \param dentry Pointer to the dentry to destroy.
 */
void vfs_dentry_destroy (vfs_dentry_t *dentry);

//--------------------------------------------------------------------------------------------------
// inode API
//--------------------------------------------------------------------------------------------------

/**
 * \brief Creater and initialize the given inode
 * \param sb    Pointer to the superblock where the inode should belong.
 * \param ino   Index of the inode to search for.
 * \param size  file size
 * \param mode  file mode : permission and type
 * \param data  file private data (depends on real file system)
 * \return Pointer to the newly created vfs_inode_t.
 */
vfs_inode_t *vfs_inode_create (superblock_t *sb, ino_t ino, size_t size, mode_t mode, void *data);

/**
 * \brief Lookup an inode in the VFS inode cache.
 * \param sb  Pointer to the superblock where the inode should belong.
 * \param ino Index of the inode to search for.
 * \return Pointer to the vfs_inode_t if found, NULL otherwise.
 */
vfs_inode_t *vfs_inode_lookup (superblock_t *sb, ino_t ino);

/**
 * \brief Increment the reference count of an inode.
 * \param inode Pointer to the inode whose reference count is incremented.
 */
void vfs_inode_get (vfs_inode_t *inode);

/**
 * \brief Decrement the reference count of an inode and free it if it reaches zero.
 * \param inode Pointer to the inode whose reference count is decremented.
 */
void vfs_inode_release (vfs_inode_t *inode);

#endif//_VFS_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
