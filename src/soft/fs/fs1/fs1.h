/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     fs/fs1/fs1.h
  \author   Franck Wajsburt
  \brief    Minimalist Read Only File System 1 directory --> FS1

\*------------------------------------------------------------------------------------------------*/

#ifndef _FS1_H_
#define _FS1_H_

/**
 * fs_ops table for fs1 (only a subset is implemented).
 * .mount  Mount the a fs1 filesystem on a given block device.
 * .lookup Lookup a file from its name in a single fs1 directory.
 * .read   Read data from a file stored in the fs1 file system. 
 */
extern vfs_fs_type_t fs1_ops;

#endif//_FS1_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
