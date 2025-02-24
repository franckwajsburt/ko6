/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-02-24
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     kfs.h
  \author   Franck Wajsburt
  \brief    kfs-lite ko6 file system lite

  FIXME  mknod, rename, rmdir, unlinkat, mkfifo, symlink

\*------------------------------------------------------------------------------------------------*/

#ifndef _KFS_H_
#define _KFS_H_

#ifndef __DEPEND__
#ifdef  _HOST_
#   include <stdlib.h>
#   include <stdio.h>
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif
#endif

//--------------------------------------------------------------------------------------------------
// Size of all disk areas in pages and types of entries
//--------------------------------------------------------------------------------------------------

#define KFS_NPG_DISK    0x2000  /* number of pages in the whole disk */
#define KFS_NPG_MBR     1       /* number of pages in MBR */
#define KFS_NPG_VBR     3       /* number of pages in VBR (bootloader) */
#define KFS_NPG_SBLOCK  2       /* number of pages in SBLOCK (contains all bitmaps) */
#define KFS_NPG_DENTRY  2       /* number of pages for all dentries */
#define KFS_NPG_INODE   2       /* number of pages for all inodes */
#define KFS_NPG_FMAP    16      /* number of pages for all file maps (inode extensions) */
#define KFS_NPG_BOOT    (KFS_NPG_MBR+KFS_NPG_VBR)
                                /* number of pages for boot part */
#define KFS_NPG_META    (KFS_NPG_SBLOCK+KFS_NPG_DENTRY+KFS_NPG_INODE+KFS_NPG_FMAP)
                                /* number of pages for all metadata */

enum kfs_type_e {               ///< type of files for the real disk (less than vfs)
    KFS_DIR,                    ///< directory
    KFS_FILE,                   ///< regular file
    KFS_PIPE,                   ///< named fifo     not yet handled #fixme
    KFS_SLINK                   ///< symbolic link  not yet handled #fixme
};

//--------------------------------------------------------------------------------------------------
// accessor functions
//--------------------------------------------------------------------------------------------------

/**
 * \brief   read access functions to dentries
 * \param   index of a dentry
 * \return  -   a dentry number for kfs_root(), kfs_next(), kfs_leaf().
 *              For these three functions, 0 means either file system root '/' or last leaf
 *          -   the inode number for kfs_inode(), there is always one
 *          -   the pointer to the entry name for kfs_name()
 */
int  kfs_root    (int dentry);
int  kfs_next    (int dentry);
int  kfs_leaf    (int dentry);
int  kfs_inode   (int dentry);
char *kfs_name   (int dentry);

/**
 * \brief   boolean functions for dentries or inode
 * \param   index of a dentry
 * \return  -   0 if false, not 0 if true
 */
int  kfs_isdir  (int dentry);

/**
 * \brief   read access functions to inodes
 * \param   index of an inode
 * \param   offset is the page number in the file (start at 0 to filesize / sizeof(page))
 * \return  -   kfs_count : number of dentry pointing to this inode
 *          -   kfs_type  : type defined in kfs_enum_e
 *          -   kfs_mode  : permission modes rwxrwx for the owner and others (there are no groups)
 *          -   kfs_size  : file size
 *          -   kfs_owner : file or directory owner
 *          -   kfs_mtime : modification / creation relative time
 *          -   kfs_page  : index of page of data on the disk
 */
int  kfs_count  (int inode);
int  kfs_type   (int inode);
int  kfs_mode   (int inode);
int  kfs_size   (int inode);
int  kfs_owner  (int inode);
int  kfs_mtime  (int inode);
int  kfs_page   (int inode, int pg_offset);
/**
 * \brief   write access functions to inode's properties
 * \param   index of an inode
 * \param   mode, owner & mtime are the new property value
 * \return  the new property or -1 if failure
 */
int kfs_chmode  (int inode, int mode);
int kfs_chowner (int inode, int owner);
int kfs_chmtime (int inode, int mtime);

//--------------------------------------------------------------------------------------------------
// functions on files or directories
//--------------------------------------------------------------------------------------------------

/**
 * \brief   opens a file or directory, if file or directory not found, all the pathname is created
 * \param   pathname is the absolute name of the file or directory to find in root
 * \return  dentry index of the file or directory, that is the file descriptor or -1 on failure
 */
int kfs_open (char *full_pathname);

/**
 * \brief   looks for name in the directory root, if the name is not found, it is created
 * \param   root is a dentry index
 * \param   dentry_name is the name of the file or directory to find in root
 * \return  dentry index of the file, that is the file descriptor
 */
int kfs_openat (int root, char *dentry_name);

/**
 * \brief   Reads a directory, it reads one entry from the directory dentry to memory buffer.
 *          After reading, buffer will contain the name of the directory or item
 * \param   dentry given by open or openat
 * \param   leaf is a dentry of an item present in the directory
 *          if leaf == -1 then the item is the one placed just after the previous one read
 * \param   buffer to put the filename
 * \return  next dentry to read
 *          or -1 if something is wrong
 */
int kfs_readdir (int dentry, int leaf, void *buffer);

/**
 * \brief   Reads a file, it reads one page from the file dentry to memory buffer.
 *          After reading, buffer will contain the entire page of which the offset belongs
 * \param   dentry given by open or openat
 * \param   pg_offset is the page of the file to be read,
 *          if offset == -1 then the item or page placed just after the previous one read
 * \param   buffer to put the read page or filename (at least one page length)
 * \return  for a file the number of pages read :
 *           1 if success with page in buffer as a side effect,
 *           0 if file smaller than offset,
 *          -1 if something is wrong
 */
int kfs_read (int dentry, int pg_offset, int *buffer);

/**
 * @brief Sets the size field of the inode associated with the dentry to newsize
 *
 * @param dentry given by open or openat
 * @param newsize new size of the file
 * @return int : the size of the file after modification
 */
int kfs_set_size(int dentry, int newsize);

/**
 * \brief   Writes one page from memory buffer to file dentry.
 *          The buffer is written on the disk in file page of which the offset belongs.
 * \param   dentry given by open or openat
 * \param   pg_offset is the number of the page that will be written
 * \param   buffer contains the page to write (at least one page length)
 * \return  the number of pages written
 *           1 if success with page in disk as a side effect,
 *           0 if success but buffer was full of 0, thus no page needs to be allocated
 *          -1 if something is wrong
 */
int kfs_write (int dentry, int pg_offset, void *buffer);

/**
 * \brief   removes a file or a directory
 * \param   pathname is the absolute name of the file or directory to find in root
 * \return  dentry index of the file or directory, that is the file descriptor or -1 on failure
 */
int kfs_remove (char *full_pathname);
int kfs_unlink (char *name);

/**
 * \brief   creates a new link (also called hard link) on an existing file.
 * \param   old_name is an existing file name
 * \param   new_name is the new name of the same file as old_name
 *          if new_name already exists it is not created, and it's a failure
 * \return  0 if success, -1 if failure
 */
int kfs_link (char *old_name, char *new_name);

/**
 * \brief   calls the callback function on each entry from root dentry
 * \param   root is the dentry for which the recursion begins
 * \param   callback is a provided function which will be called on each dentry
 *          - dentry is the current entry in this directory which can be of any type
 *          - depth gives the callback() function information about the depth of the recursion.
 *            the depth of the root is 0, then the first level is 1, etc.
 *          - position is the current position in the current directory (0, 1, etc.)
 * \return  the number of calls of the callback function
 */
int kfs_tree_cb (int root, void (*callback)(int dentry, int depth, int position));

/**
 * \brief Loads a 512 bytes sized MBR in kfs. Meant to be used exclusively on Linux.
 * \param pathname the pathname on host to the mbr
 * \return the number of bytes read
 */
int kfs_add_mbr(char* pathname);

/**
 * \brief Loads a VBR in kfs. Meant to be used exclusively on Linux.
 * \param pathname the pathname on host to the vbr
 * \return the number of bytes read
 */
int kfs_add_vbr(char* pathname);

/**
 * \brief   Loads the disk from an external support.
 *          On HOST     It means from the host disk in file whose name is given in parameter.
 *                      After load, the whole content of the disk is present in memory
 *                      in tables KfsMbr, KfsVbr, KfsSblock, KfsDentry, KfsInode, KfsFmap & KfsDisk.
 *          On ko6      It means from the virtual disk connected to the Block Device controller.
 *                      After load, only METADATA content of the disk is present in memory
 *                      in tables KfsSblock, KfsDentry, KfsInode, KfsFmap
 * \param   filename parameter only used by the HOST
 * \return  a number > 0 on success, and <= 0 on failure
 */
int kfs_disk_load(char *pathname);

/**
 * \brief   Saves the disk to an external support.
 *          On HOST     It means to the host disk in a file whose name is given in parameter.
 *                      After save, the whole content of the disk is written in file on host.
 *                      That is KfsMbr, KfsVbr, KfsSblock, KfsDentry, KfsInode, KfsFmap & KfsDisk.
 *          On ko6      It means to file connected to the Block Device controller.
 *                      After save only METADATA content of the disk is written to the virtual disk.
 *                      That is KfsSblock, KfsDentry, KfsInode, KfsFmap.
 * \param   filename parameter only used by the HOST
 * \return  a number > 0 on success, and <= 0 on fealure
 */
int kfs_disk_save(char *pathname);

#endif
