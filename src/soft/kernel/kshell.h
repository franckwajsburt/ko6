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

#ifndef _KSHELL_H_
#define _KSHELL_H_

#include <common/kshell_syscalls.h>

int sys_kshell (kshell_syscall_t service, kshell_args_t *args);

#endif//_KSHELL_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
