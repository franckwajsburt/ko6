/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-05-03
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     commmon/errno.c
  \author   Franck Wajsburt
  \brief    Standard error messages (https://man7.org/linux/man-pages/man3/errno.3.html)

\*------------------------------------------------------------------------------------------------*/

#include <common/errno.h>

/*
 * the +1 is because FAILURE == -1, thus we need to shift code of 1
 */
char * errno_mess_table[] = {
   [FAILURE +1] = "Any error",
   [SUCCESS +1] = "No error",
   [E2BIG   +1] = "Argument list too long",
   [EACCES  +1] = "Permission denied",
   [EAGAIN  +1] = "Resource temporarily unavailable",
   [EBUSY   +1] = "Device or resource busy",
   [EDEADLK +1] = "Ressource deadlock avoided",
   [EFAULT  +1] = "Bad address",
   [EFBIG   +1] = "File too large",
   [EINTR   +1] = "Interrupted system call",
   [EINVAL  +1] = "Invalid argument",
   [EIO     +1] = "Input/output error",
   [EBADF   +1] = "Bad file descriptor",
   [EISDIR  +1] = "Operation forbidden on a directory",
   [EEXIST  +1] = "File or directory already exist",
   [ENOBUFS +1] = "No buffer space available",
   [ENODEV  +1] = "No such device",
   [ENOMEM  +1] = "Not enough space/cannot allocate memory",
   [ENOSPC  +1] = "No space left on device",
   [ENOSYS  +1] = "Function not implemented",
   [ENOTTY  +1] = "Inappropriate I/O control operation",
   [ENOTDIR +1] = "File is not a directory",
   [ENOENT  +1] = "Entry not found",
   [ENOEXEC +1] = "Not an executable",
   [ENXIO   +1] = "No such device or address",
   [EPERM   +1] = "Operation not permitted",
   [ERANGE  +1] = "Math result not representable",
   [ESRCH   +1] = "No such thread or Process",
   [EROFS   +1] = "Read-only file system"
};
