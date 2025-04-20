/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     kernel/kinit.c
  \author   Franck Wajsburt
  \brief    kernel initilisation function
            - This function is called at the end of boot code.
            - Its role is:
              - to display the ko6 banner
              - to initialise the memory allocators
              - to initialise the SoC (thanks to the soc-specific function soc_init())
              - to initialise the thread scheduler
              - to create and launch the first user process

\*------------------------------------------------------------------------------------------------*/

#include <kernel/klibc.h>

#define TICK    200000

void kinit (void *fdt)
{
    // Hardware and structure inialization

    kmemkernel_init ();                         // kernel mem initialization, do it before soc_init
    PANIC_IF (soc_init (fdt, TICK) < 0, "SoC initialization failed");
    kmemuser_init ();                           // user memory initialization 
    kprintf (Banner_ko6);                       // ko6 banner
    ksynchro_init ();                           // initialize all synchronization mecanisms

    // Then, create the thread structure for the thread main()
    //   thread_create() is the same function used to create the thread main()
    //   and to create an usual thread, here, we create the the thread main, it takes 4 args
    //   1. The thread structure has to be placed in the .kdata segment but the thread identifier 
    //      has to be placed in the .data segment (user space) at a place known by the kernel.
    //      That's why, the thread identifier of main() is placed at the very beginning of .data
    //   2. The address of the thread function (cast to int), here it should be the address of
    //      main function, but we are not able to know this address because it is in the user
    //      segment .text, thus we put 0 and it is the user function _start() which call main()
    //      function by principle (the main function is always mandatory).
    //   3. The argument of the thread function (cast to int), in that case we does not know it,
    //      thus we put 0, and it is the user function _start() that will find the arguments
    //      of main(). Note that in our case, there is not arguments for the function main().
    //   4. The address of the main() starter, nammed _start(), it is this function that will
    //      call the main() function with its arguments (here none) and then will call exit()
    //      when main() returns. Note, the starter of an usual thread is thread_start() (defined
    //      in thread.c) and it  will call thread_exit() when the thread returns.

    thread_create ((thread_t *)&__usermem.main_thread, 0, 0, (int)__usermem.main_start);

    // Next, we have to load the context of main() and start it
    //   It means to initialize the stack pointer, the status register and the return address ($31)
    //   And, as it it is the first load, jr $31 will go to thread_bootstrap which go to _start()
    //   function in user mode. You can look at the comment of the bootstrap() function in
    //   kthread.c file for details.

#   if 0
    kmalloc_stat ();
    kmalloc_test (100, 2048);
    test_ustack (10);
#   endif

    //blockio_init ();
    
    // Finally, load the main user programm
    // We never return of thread_load() here because thread_load() change $31 to thread_bootstap()

    thread_main_load (__usermem.main_thread);
    PANIC_IF(true,"Impossible to be here");
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
