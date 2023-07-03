/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-03
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kinit.c
  \author   Franck Wajsburt
  \brief    kernel initilisation function
            - This function is called at the end of boot code.
            - Its role is:
              - to display the kO6 banner
              - to initialise the memory allocators
              - to initialise the SoC (thanks to the soc-specific function arch_init())
              - to initialise the thread scheduler
              - to create and launch the first user process

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>

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

void kinit (void)
{
    kprintf (Banner);
    
    extern int __dtb_address;
    char *fdt = (char*) &__dtb_address;

    if (*(unsigned int*) fdt == 0xedfe0dd0) {
        kprintf("Device tree found at %p\n", fdt);
    } else {
        kprintf("No device tree found\n");
    }

    // put bss sections to zero. bss contains uninitialised global variables
    extern int __bss_origin;    // first int of bss section (defined in ldscript kernel.ld)
    extern int __bss_end;       // first int of above bss section (defined in ldscript kernel.ld)
    for (int *a = &__bss_origin; a != &__bss_end; *a++ = 0);

    memory_init();                  // memory initialisation 
    arch_init(200000);              // architecture initialisation takes the tick as argument

    // First, we have to create the thread structure for the thread main()
    //   thread_create() is the same function used to create the thread main()
    //   and to create an usual thread, here, we create the the thread main, it takes 4 args
    //   1. The address of the thread structure,
    //      This structure must be placed in the .data segment (user space) because there is
    //      the stack inside but the address of the the structure must be known by the kernel.
    //      That's why, the thread structure of main() is placed at the very beginning of .data
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

    extern thread_t _main_thread;   // main thread identifier (see kernel.ld)
    extern int _start;              // _start is the entry point of the app (see kernel.ld)
    thread_create (&_main_thread, 0, 0, (int)&_start);

    // Next, we have to load the context of main() and start it, this replaces app_load()
    //   It means to initialize the stack pointer, the status register and the return address ($31)
    //   And, as it it is the first load, jr $31 will go to thread_bootstrap which go to _start()
    //   function in user mode. You can look at the comment of the bootstrap() function in
    //   kthread.c file for details.

    thread_main_load (_main_thread);

    // We never return of thread_load() here because thread_load() change $31 to thread_bootstap()
    PANIC_IF(true,"Impossible to be here");
}
