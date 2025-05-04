/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/errno.h
  \author   Franck Wajsburt
  \brief    Standard error numbers 

  This header defines:
  * A standard list of error codes consistent with POSIX conventions.
      See https://man7.org/linux/man-pages/man3/errno.3.html
  * A thread-local mechanism to access the current error (errno).
  * A typedef (errno_t) used to make error-returning functions more explicit.

  Note:
  * All functions returning an errno_t use negative values to indicate errors (e.g., -EINVAL).
  * A return value of 0 means success (SUCCESS).
  * This enum is typedef’d as `errno_t` for better code clarity, even though it is just an int.
  
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
 * \brief errno_mess contains the error messages for each error code
 *        this table is define in both files libc.c and klib.c but should be identical
 */
extern char * errno_mess_table[];
#define errno_mess(err)  errno_mess_table[1+(err)]

/**
 * \brief Typedef for error codes, used to make function signatures more expressive.
 */
typedef enum errno_code errno_t;

enum errno_code {
    FAILURE = -1, ///< Generic failure (used when no specific errno is set)
    SUCCESS,      ///< No error (operation succeeded)
 
    // Argument or permission errors
    E2BIG,        ///< Argument list too long
    EACCES,       ///< Permission denied
    EAGAIN,       ///< Resource temporarily unavailable
    EBADF,        ///< Bad file descriptor
    EEXIST,       ///< File or directory already exists
    EFAULT,       ///< Bad address
    EINVAL,       ///< Invalid argument
    EPERM,        ///< Operation not permitted
    EROFS,        ///< Read-only file system

    // System or resource limits
    ENOMEM,       ///< Not enough space/cannot allocate memory
    ENOBUFS,      ///< No buffer space available
    EFBIG,        ///< File too large
    ENOSPC,       ///< No space left on device
    EDEADLK,      ///< Resource deadlock avoided
    ERANGE,       ///< Result too large (overflow)

    // File system / I/O errors
    ENOENT,       ///< file/directory not found
    ENOTDIR,      ///< file is not a directory
    EISDIR,       ///< operation forbidden on a directory
    ENODEV,       ///< No such device
    ENXIO,        ///< No such device or address
    ENOEXEC,      ///< Not an executable
    ENOTTY,       ///< Inappropriate I/O control operation
    EIO,          ///< Input/output error
    EBUSY,        ///< Device or resource busy

    // Signals / processes
    EINTR,        ///< Interrupted funct call (https://man7.org/linux/man-pages/man7/signal.7.html)
    ESRCH,        ///< No such process

    // Unimplemented features
    ENOSYS        ///< Function not implemented
};


#endif//_ERRNO_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
