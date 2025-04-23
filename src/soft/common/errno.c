/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
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
char * errno_mess[] = {
   [FAILURE +1] = "any error",
   [SUCCESS +1] = "no error",
   [E2BIG   +1] = "Argument list too long",
   [EACCES  +1] = "Permission denied",
   [EAGAIN  +1] = "Resource temporarily unavailable",
   [EBUSY   +1] = "Device or resource busy",
   [EDEADLK +1] = "Ressource dead lock avoided",
   [EFAULT  +1] = "Bad address",
   [EFBIG   +1] = "File too large",
   [EINTR   +1] = "Interrupted system call",
   [EINVAL  +1] = "Invalid argument",
   [EIO     +1] = "I/O error",
   [EBADF   +1] = "Bad file descriptor",
   [EISDIR  +1] = "operation forbidden on a directory",
   [EEXIST  +1] = "file/directory already exist",
   [ENOBUFS +1] = "No buffer space available",
   [ENODEV  +1] = "No such device",
   [ENOMEM  +1] = "Out of memory",
   [ENOSPC  +1] = "No space left on device",
   [ENOSYS  +1] = "Function not implemented",
   [ENOTTY  +1] = "Not a typewriter",
   [ENOTDIR +1] = "file is not a directory",
   [ENOENT  +1] = "file/directory not found",
   [ENOEXEC +1] = "not an executable",
   [ENXIO   +1] = "No such device or address",
   [EPERM   +1] = "Operation not permitted",
   [ERANGE  +1] = "Math result not representable",
   [ESRCH   +1] = "Thread or Processus non-existent"
};
