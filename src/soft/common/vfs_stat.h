/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-24
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     vfs_stat.h
  \author   Franck Wajsburt
  \brief    Defines types, modes and permission flags for VFS and user applications.

  This header provides: 
  
  * Basic types used for file system metadata, compatible with POSIX conventions.
    These types are shared between the kernel and user space (libc), 
    and used mainly for struct stat and related system calls.
 
  * Structure containing file attributes used by VFS API function (similar to POSIX struct stat).
  
  * Symbolic constants used to describe file types and permissions,
    compatible with POSIX macros (S_IFREG, S_IRUSR, etc.). 
    It is intended for both kernel and userland use in ko6.
    The values are POSIX, but it doesn't matter

\*------------------------------------------------------------------------------------------------*/

#ifndef _VFS_STAT_H_
#define _VFS_STAT_H_

//--------------------------------------------------------------------------------------------------
// Basic types
//--------------------------------------------------------------------------------------------------

typedef unsigned int mode_t;   ///< File mode (type + permissions)
typedef unsigned int ino_t;    ///< Inode number
typedef unsigned int nlink_t;  ///< Number of hard links
typedef unsigned int uid_t;    ///< User ID
typedef unsigned int dev_t;    ///< Device ID (major and minor combined)
typedef unsigned long off_t;   ///< File size or offset
typedef unsigned long time_t;  ///< Time in seconds since UNIX epoch

//--------------------------------------------------------------------------------------------------
// Structure containing file attributes 
//--------------------------------------------------------------------------------------------------

struct stat {
    dev_t     st_dev;     ///< Device ID containing the file (minor number of the blockdev)
    ino_t     st_ino;     ///< Inode number
    mode_t    st_mode;    ///< File type and permissions
    nlink_t   st_nlink;   ///< Number of hard links
    uid_t     st_uid;     ///< User ID of owner
    dev_t     st_rdev;    ///< Device ID (S_IFCHR: chardev's minor nb; S_IFBLK: blockdev's minor nb)
    off_t     st_size;    ///< Total size, in bytes
    time_t    st_atime;   ///< Time of last access
    time_t    st_mtime;   ///< Time of last modification
    time_t    st_ctime;   ///< Time of last status change
};

//--------------------------------------------------------------------------------------------------
// modes & permissions
//--------------------------------------------------------------------------------------------------

#define S_IFMT   0xF000                                     ///< Bitmask for the file type field

#define S_IFREG  0x8000                                     ///< Inode Flag: Regular file
#define S_IFDIR  0x4000                                     ///< Inode Flag: Directory
#define S_IFCHR  0x2000                                     ///< Inode Flag: Character device
#define S_IFBLK  0x6000                                     ///< Inode Flag: Block device
#define S_IFIFO  0x1000                                     ///< Inode Flag: FIFO (named pipe)
#define S_IFLNK  0xA000                                     ///< Inode Flag: Symbolic link
#define S_IFSOCK 0xC000                                     ///< Inode Flag: Socket

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)             ///< is a regular file?
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)             ///< is a directory?
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)             ///< is a character device?
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)             ///< is a block device?
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)             ///< is a FIFO?
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)             ///< is a symbolic link?
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)            ///< is a socket?

#define S_IXOTH  0x0001                                     ///< others permission: eXecute/search
#define S_IWOTH  0x0002                                     ///< others permission: Write
#define S_IROTH  0x0004                                     ///< others permission: Read

#define S_IXUSR  0x0040                                     ///< owner permission: Execute/search
#define S_IWUSR  0x0080                                     ///< owner permission: Write
#define S_IRUSR  0x0100                                     ///< owner permission: Read

#define S_IRWXOTH  (S_IROTH|S_IWOTH|S_IXOTH)                ///< others permission: RWX
#define S_IRWXUSR  (S_IRUSR|S_IWUSR|S_IXUSR)                ///< owner permission: RWX
#define S_IRWXALL  (S_IRWXOTH|S_IRWXUSR)                    ///< permission: RWX for all
                    
#endif//_VFS_STAT_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
