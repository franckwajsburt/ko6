/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-06-17
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     ulib/crt0.c
  \author   Franck Wajsburt
  \brief    Application starter

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>
#include <pthread.h>

extern int __bss_origin;        // first int of uninitialized global data
extern int __bss_end;           // first int of above the uninitialized global data
extern int __data_end;          // last address of the free user data region
extern int main (void);         // tell the compiler that main() exists

__attribute__((section (".usermem")))
usermem_t _user_mem = {
    .ustack_end   = &__data_end,// at the begining there is no stack.
    .ustack_beg   = &__data_end,// last address of the free user data region
    .uheap_beg    = &__bss_end ,// first address of the free user data region 
    .uheap_end    = &__bss_end  // at the begining there is no heap.
};   

// in kinit.c, the main thread is created, then loaded :
// - thread_create_kernel (&_main_thread, 0, 0, (int)&_start);
// - thread_load (((kthread_t*)_main_thread)->context);

__attribute__((section (".main_thread")))
pthread_t _main_thread;

__attribute__((section (".start")))
void _start (void)
{
    int res;
    for (int *a = &__bss_origin; a != &__bss_end; *a++ = 0);
    malloc_init (_user_mem.uheap_beg);
    res = main ();
    exit (res);
}
