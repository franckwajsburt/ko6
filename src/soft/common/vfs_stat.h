/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-24
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     vfs_stat.h
  \author   Franck Wajsburt
  \brief    Defines file type and permission flags for VFS and user applications.

  This header provides symbolic constants used to describe file types and permissions,
  compatible with POSIX macros (S_IFREG, S_IRUSR, etc.). 
  It is intended for both kernel and userland use in ko6.
  The values are POSIX, but it doesn't matter

\*------------------------------------------------------------------------------------------------*/

#ifndef _VFS_STAT_H_
#define _VFS_STAT_H_

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
                    
#endif//_VFS_STAT_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
