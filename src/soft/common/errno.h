/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/errno.h
  \author   Franck Wajsburt
  \brief    Standard error numbers (https://man7.org/linux/man-pages/man3/errno.3.html)

\*------------------------------------------------------------------------------------------------*/

#ifndef _ERRNO_H_
#define _ERRNO_H_

/**
 * \brief   errno is a thread local variable, that means it is like a global variable
 *          (thus accessible anywhere) but each thread has its own value.
 *          To do that errno is a define that is replaced by *__errno_location()
 *          __errno_location() gives an address different for each thread
 *          *__errno_location() dereferences the pointeur,  i.e. follows the pointer
 *          and it designates what is pointed to.
 */

#ifdef _KERNEL_     /* _KERNEL_ is defined at the beginning of kernel/klibc.h  */
#   define errno    *thread_errno(ThreadCurrent)    /* gets errno from the tls of ThreadCurrent */
#else
#   define errno    (__usermem.ptls->tls_errno)     /* gets errno of the current running thread */
#endif

/**
 * errno_mess contains the error messages for each error code
 * this table is define in both files libc.c and klib.c but should be identical
 */
extern char * errno_mess[];

enum errno_code {
    FAILURE = -1,
    SUCCESS ,
    E2BIG   ,   // Argument list too long 
    EACCES  ,   // Permission denied 
    EAGAIN  ,   // Resource temporarily unavailable
    EBUSY   ,   // Device or resource busy
    EDEADLK ,   // Resource deadlock avoided
    EFAULT  ,   // Bad address
    EFBIG   ,   // File too large
    EINTR   ,   // Interrupted function call (https://man7.org/linux/man-pages/man7/signal.7.html)
    EINVAL  ,   // Invalid argument
    EIO     ,   // Input/output error
    EBADF   ,   // Bad file descriptor
    EISDIR  ,   // operation forbidden on a directory
    EEXIST  ,   // file/directory already exist
    ENOBUFS ,   // No buffer space available
    ENODEV  ,   // No such device
    ENOMEM  ,   // Not enough space/cannot allocate memory
    ENOSPC  ,   // No space left on device
    ENOSYS  ,   // Function not implemented
    ENOTTY  ,   // Inappropriate I/O control operation
    ENOTDIR ,   // file is not a directory
    ENOENT  ,   // file/directory not found
    ENOEXEC ,   // not an executable
    ENXIO   ,   // No such device or address
    EPERM   ,   // Operation not permitted
    ERANGE  ,   // Result too large
    ESRCH       // No such process
};

#endif//_ERRNO_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
