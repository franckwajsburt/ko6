/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-27
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     ulib/crt0.c
  \author   Franck Wajsburt
  \brief    Application starter

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>
#include <pthread.h>

extern int __bss_origin;        // first int of uninitialized global data of app0
extern int __bss_end;           // first int of above the uninitialized global data of app0
extern int __data_end;          // last address of the free user data region of app0
extern int main (void);         // tell the compiler that main() exists
static void _start (void);      // very start function of the process

__attribute__((section (".usermem")))
__usermem_t __usermem = {
    .ustack_end = &__data_end,  // at the beginning there is no stack.
    .ustack_beg = &__data_end,  // last address of the free user data region
    .uheap_beg  = &__bss_end ,  // first address of the free user data region
    .uheap_end  = &__bss_end ,  // at the beginning there is no heap.
    .main_thread = NULL      ,  // thread pointer initialized by thread_create() in kernel/kinit.c
    .main_start  = _start       // _start function of the process
};

void _start (void)
{
    int res;
//    urandseed = 1;
    for (int *a = &__bss_origin; a != &__bss_end; *a++ = 0);
    memset (&__bss_origin, 0, __bss_end - __bss_origin);
    malloc_init (__usermem.uheap_beg);
    res = main ();
    exit (res);
}
