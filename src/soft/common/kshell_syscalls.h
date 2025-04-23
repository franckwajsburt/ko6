/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/kshell_syscalls.h
  \author   Lili Zheng, Marco Leon, Franck Wajsburt
  \brief    kshell syscall interface

  There is only one kshell syscall which dispatch the real kernel services.
  * arg1: service number 
  * arg2: structure pointer for service arguments (union) 
  * arg3: structure pointer for results 
  * arg4: debug structure pointer with counters or status (optionnal)

\*------------------------------------------------------------------------------------------------*/

#ifndef _KSHELL_SYSCALLS_H_
#define _KSHELL_SYSCALLS_H_

//--------------------------------------------------------------------------------------------------
// \brief kshell service number : used in uapp/kshell.c and kernel/kshell.c
//--------------------------------------------------------------------------------------------------

typedef enum {
    KSHELL_OPEN = 0,        ///< open file or directory
    KSHELL_READ,            ///< read file
    KSHELL_WRITE,           ///< write file
    KSHELL_CLOSE,           ///< close file
    KSHELL_UNLINK,          ///< destroy file, if not used

    KSHELL_MKDIR,           ///< add a new directoty
    KSHELL_CHDIR,           ///< change the working directory to allow relative path
    KSHELL_RMDIR,           ///< destroy a directory, if not used and if empty
    KSHELL_READDIR,         ///< read next item

    KSHELL_CLONE,           ///< fork/exec to lauch a new user app
    KSHELL_KILL,            ///< send signal to the user app

    KSHELL_SYSCALL_NR       ///< number of kshell services
} kshell_syscall_t;

//--------------------------------------------------------------------------------------------------
// \brief arguments and results for kshell service, each sub-service has its own arguments/result
//--------------------------------------------------------------------------------------------------


typedef struct {            // ---------------------------------------------------------------- open
    const char *path;       // absolute or relative pathname
    int flags;              // Defined above
    int resfd;              // result: file descriptor
    int error;              // error code: ENOENT, EEXIST, EISDIR, ENOTDIR, EINVAL, EIO
} kshell_open_t;

#define O_RDONLY    0x0001  // Acess mode : Open for reading only
#define O_WRONLY    0x0002  // Acess mode : Open for writing only
#define O_RDWR      0x0003  // Acess mode : Open for reading and writing

#define O_CREAT     0x0010  // Access behavior : Create the file if it does not exist
#define O_EXCL      0x0020  // Access behavior : Fail if the file already exists (with O_CREAT)
#define O_APPEND    0x0040  // Access behavior : Append data to the end of the file on each write

#define O_DIRECTORY 0x0100  // Directory : Fail if the path is not a directory

typedef struct {            // ---------------------------------------------------------------- read
    int fd;                 // file descriptor return by open
    void *buf;              // buffer to receive data read
    size_t count;           // number of bytes to read
    int error;              // error code: EBADF, EISDIR, EIO
} kshell_read_t;

typedef struct {            // --------------------------------------------------------------- write
    int fd;                 // file descriptor return by open
    const void *buf;        // buffer with data to write
    size_t count;           // number of bytes to write
    int error;              // error code: EBADF, EISDIR, ENOSPC, EIO
} kshell_write_t;

typedef struct {            // --------------------------------------------------------------- close
    int fd;                 // file descriptor return by open
    int error;              // error code: EBADF
} kshell_close_t;

typedef struct {            // -------------------------------------------------------------- unlink
    const char *path;       // absolute or relative pathname
    int error;              // error code: ENOENT, EISDIR, EIO
} kshell_unlink_t;

typedef struct {            // --------------------------------------------------------------- mkdir
    const char *path;       // absolute or relative pathname
    int error;              // error code: EEXIST, ENOTDIR, EINVAL, ENOSPC, ENOMEM, EIO
} kshell_mkdir_t;

typedef struct {            // --------------------------------------------------------------- chdir
    const char *path;       // absolute or relative pathname
    int error;              // error code: ENOENT, ENOTDIR
} kshell_chdir_t;

typedef struct {            // --------------------------------------------------------------- rmdir
    const char *path;       // absolute or relative pathname
    int error;              // error code: ENOENT, ENOTDIR
} kshell_rmdir_t;

typedef struct {            // ------------------------------------------------------------- readdir
    int fd;                 // file descriptor return by open
    void *entry_buf;        // buffer for a single entry
    size_t entry_buf_sz;    // buffer size
    int error;              // error code: EBADF, EINVAL, EIO
} kshell_readdir_t;

typedef struct {            // --------------------------------------------------------------- clone
    const char *path;       // absolute or relative pathname
    char **argv;            // pointer to argument (char*) table 
    char **envp;            // pointer to environment variable (char*) table 
    int pid;                // result pid: number
    int error;              // error code: ENOENT, ENOEXEC, ENOMEM, EBUSY
} kshell_clone_t;

typedef struct {            // ---------------------------------------------------------------- kill
    int pid;                // process to kill, even if there is only one
    int signal;             // signal number
    int error;              // error code: ERSRCH, EPERM, EINVAL
} kshell_kill_t;

typedef union {             // union of all
    kshell_open_t    a_open;
    kshell_read_t    a_read;
    kshell_write_t   a_write;
    kshell_close_t   a_close;
    kshell_unlink_t  a_unlink;
    kshell_mkdir_t   a_mkdir;
    kshell_chdir_t   a_chdir;
    kshell_rmdir_t   a_rmdir;
    kshell_readdir_t a_readdir;
    kshell_clone_t   a_clone;
    kshell_kill_t    a_kill;
} kshell_args_t;

#endif // KSHELL_SYSCALLS_H

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
