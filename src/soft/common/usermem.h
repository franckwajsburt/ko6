/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     common/usermem.h
  \author   Franck Wajsburt
  \brief    User application memory description
            main thread pointers: tid and main start function (TODO thread start ??)
            thread Local Storage declaration
            in the future: argc, argv, arge

\*------------------------------------------------------------------------------------------------*/

#ifndef _USERMEM_H_
#define _USERMEM_H_

#define PAGE_SIZE       4096            ///< page size, can't be changed
#define USTACK_SIZE     (16*PAGE_SIZE)  ///< thread stack size (all thread have the same size)
#define MAGIC_STACK     0xDEADF00D      ///< used to tag user stack (to check corruption)
#define MAGIC_HEAP      0x5A            ///< used to tag user heap block (to check corruption)
#define MAX_O_FILE      64              ///< file descriptor of the open files

/**
 * \brief few data about user memory usage, stucture placed at the bottom of the .data region
 *
 * +-------------+
 * | tls of main |
 * | - - - - - - | <- ustack_begin  points to the first address ABOVE the first user stack
 * | user  stack |                  The last word of each stack contains also a MAGIC number
 * | thread main |                  in order to be able to know if the stack has overflowed
 * +-------------+                  This check must be done sometimes because
 * | tls of th 1 |                  there is not MMU to check it at each access
 * | - - - - - - |
 * | user  stack |
 * |  thread 1   |
 * +-------------+ <- ustack_end    points to the last cell of the last user stack
 * |/////////////|                  then ustack_end points inside of the last stack
 * |/////////////|
 * +-------------+ <- uheap_end     points to the very first address ABOVE the user heap
 * |             |                  uheap_end is always inferior or equal to stack_end
 * |  user heap  |                  uheap_end is moved with SYSCALL_BRK
 * |             |
 * +-------------+ <- uheap_begin   points to the very first address of the user heap
 * | global vars |                  all global variables of user application
 * | - - - - - - |
 * | __usermem   |                  usermem structure defined below 
 * +-------------+
 */
typedef struct _usermem_s {
    int * ustack_beg;           ///< highest address of the user stack segment
    int * ustack_end;           ///< lowest address of the user stack segment
    int * uheap_end;            ///< highest address of the user heap segment (also named brk)
    int * uheap_beg;            ///< lowest highest address of the user heap segment
    void (* main_start)(void);  ///< pointer to the start of main thread (see ulib/crt0.c)
    void * main_thread;         ///< address of the main thread (defined in kernel/kinit.c)
    struct _tls_s * ptls;       ///< pointer to the thread local storage of the current thread
    struct file_s *o_file[MAX_O_FILE];     ///< open files; 
} __usermem_t;

extern __usermem_t __usermem;   // user mem space struct (declared kernel.ld ; init in ulib/crt0.c)

/**
 * \brief thread local storage structure definition.
 *        There is one structure per thread placed at the beginning of each thread's user stack.
 *        For each field, a #define allows to hide the way of these variables are built.
 *        The current structure for the current thread is pointed by _usermem.tls,
 *        the kernel has to update _usermem.tls field at each thread commutation
 *        TODO: we'll have to extend the usage of TLS variables
 */
typedef struct _tls_s {
    int         tls_errno;      ///< syscall error number
    long long   tls_randseed;   ///< user random seed
} _tls_t;

#define urandseed   (__usermem.ptls->tls_randseed)

#endif//_USERMEM_H_

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
