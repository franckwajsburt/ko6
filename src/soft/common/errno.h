/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/errno.h
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
#define errno       *__errno_location()
 */

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
    ENOBUFS ,   // No buffer space available
    ENODEV  ,   // No such device
    ENOMEM  ,   // Not enough space/cannot allocate memory
    ENOSPC  ,   // No space left on device
    ENOSYS  ,   // Function not implemented
    ENOTTY  ,   // Inappropriate I/O control operation
    ENXIO   ,   // No such device or address
    EPERM   ,   // Operation not permitted
    ERANGE  ,   // Result too large
    ESRCH       // No such process
};

#endif//_ERRNO_H_
