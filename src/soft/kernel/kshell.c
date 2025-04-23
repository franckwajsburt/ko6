/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     kshell.c
  \author   Lili Zheng, Marc Leon, Franck Wajsburt
  \brief    Central syscall dispatcher for all user-level shell services

  The sys_kshell() function handles all shell-related system services through a unified entry point.
  Each service is identified by a numeric code and handled via a switch-case dispatch.
  The syscall interface takes a service code defined in common/kshell_syscall.h and a pointer 
  to a unified structure containing both input arguments and output results
  
\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>

char * syscall_name[] = {
    [KSHELL_OPEN]    = "KSHELL_OPEN"   ,
    [KSHELL_READ]    = "KSHELL_REAR"   ,
    [KSHELL_WRITE]   = "KSHELL_WRITE"  ,
    [KSHELL_CLOSE]   = "KSHELL_CLOSE"  ,
    [KSHELL_UNLINK]  = "KSHELL_UNLINK" ,
    [KSHELL_MKDIR]   = "KSHELL_MKDIR"  ,
    [KSHELL_CHDIR]   = "KSHELL_CHDIR"  ,
    [KSHELL_RMDIR]   = "KSHELL_RMDIR"  ,
    [KSHELL_READDIR] = "KSHELL_READDIR",
    [KSHELL_CLONE]   = "KSHELL_CLONE"  ,
    [KSHELL_KILL]    = "KSHELL_KILL"   
}; 

#define P(...) kprintf(__VA_ARGS__)
//#define P(...)

int sys_kshell (kshell_syscall_t service, kshell_args_t *args)
{
    if (service >= KSHELL_SYSCALL_NR) { 
        kprintf ("kshell error: service unknown %d\n", service);
        return  errno = ENOSYS;
    }
    kprintf ("kshell %s\t:\n", syscall_name[service]);
    switch (service) {
    case KSHELL_OPEN:                                       // open file or directory
        P("%s: path %s flag %d\n", 
            syscall_name[service], args->a_open.path, args->a_open.flags);
        args->a_open.resfd = 3;
        pvfs_init();
        O_FILE[3] = kmalloc(128);
    break;
    case KSHELL_READ:                                       // read file
    break;
    case KSHELL_WRITE:                                      // write file
    break;
    case KSHELL_CLOSE:                                      // close file
    break;
    case KSHELL_UNLINK:                                     // destroy file, if not used
    break;
    case KSHELL_MKDIR:                                      // add a new directoty
    break;
    case KSHELL_CHDIR:                                      // change the working dir
    break;
    case KSHELL_RMDIR:                                      // destroy a directory if possible
    break;
    case KSHELL_READDIR:                                    // read next item
    break;
    case KSHELL_CLONE:                                      // fork/exec to lauch a new user app
    break;
    case KSHELL_KILL:                                       // send signal to the user app
    break;
    default:
        return errno = ENOSYS;
    }
    return errno = SUCCESS;
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
