/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-23
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

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

#define Y       EC_BOLD EC_WHITE"'"EC_YELLOW"v"EC_WHITE"'"EC_RESET EC_CYAN
#define X       EC_ORANGE"x"EC_CYAN
#define X___X   " " X "___" X " "
static char Banner[] =          // banner's text defined on several lines
EC_WHITE
"   _   "  EC_CYAN"  ___  "  EC_WHITE"  __ \n"
"  | |__"  EC_CYAN" /"Y"\\ " EC_WHITE" / / \n"
"  | / /"  EC_CYAN"(     )"  EC_WHITE"/ _ \\\n"
"  |_\\_\\"EC_CYAN  X___X    EC_WHITE"\\___/\n\n"
EC_RESET;

void kinit (void *fdt)
{
    // put bss sections to zero. bss contains uninitialised global variables
    extern int __bss_origin;    // first int of bss section (defined in ldscript kernel.ld)
    extern int __bss_end;       // first int of above bss section (defined in ldscript kernel.ld)
    for (int *a = &__bss_origin; a != &__bss_end; *a++ = 0);
    memory_init();                  // memory initialisation 

    if (soc_init(fdt, 200000) < 0)  // soc initialisation takes the tick as argument
        goto sleep;                 // initialization failed, just sleep

    kprintf (Banner);

    // initialize all synchronization mecanisms

    ksynchro_init();

    // First, we have to create the thread structure for the thread main()
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

    thread_main_load (__usermem.main_thread);

    // We never return of thread_load() here because thread_load() change $31 to thread_bootstap()
    PANIC_IF(true,"Impossible to be here");

    // In case anything goes wrong during initialization
sleep:
    while (1) ;
}
