/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-02-18
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/kernel.ld
  \author   Franck Wajsburt
  \brief    kernel ldscript (SoC dependant)

\*------------------------------------------------------------------------------------------------*/

/* Description of the physical memory map
 * -------------------------------------------------------------------------------------------------
 */

/* Memory segments definition */

__boot_origin    = 0x80000000 ;     /* first byte address of boot region */
__boot_length    = 0x00001000 ;     /* boot region size */
__ktext_origin   = 0x80001000 ;     /* first byte address of kernel code region */
__ktext_length   = 0x00020000 ;
__kdata_origin   = 0x80021000 ;     /* first byte address of kernel data region */
__kdata_length   = 0x003E0000 ;
__kdata_end      = __kdata_origin + __kdata_length ; /* first addr after kernel data region */
__text_origin    = 0x81000000 ;     /* first byte address of user code region */
__text_length    = 0x00100000 ;
__data_origin    = 0x81100000 ;     /* first byte address of user data region */
__data_length    = 0x00B00000 ;
__data_end       = __data_origin  + __data_length  ; /* first addr after user data region */

_start           = __text_origin;   /* address where _start() function is expected */
_user_mem        = __data_origin;   /* address where free data segment is described */
_main_thread     = _user_mem + 16;  /* must be _user_mem + sizeof (_user_mem) */

MEMORY {
    boot_region  : ORIGIN = __boot_origin,  LENGTH = __boot_length
    ktext_region : ORIGIN = __ktext_origin, LENGTH = __ktext_length
    kdata_region : ORIGIN = __kdata_origin, LENGTH = __kdata_length
    text_region  : ORIGIN = __text_origin,  LENGTH = __text_length
    data_region  : ORIGIN = __data_origin,  LENGTH = __data_length
}

/* Description of the output sections and in which region they are put
 * -------------------------------------------------------------------------------------------------
 */

SECTIONS {
    .boot : {
        *(.boot)            /* boot code in boot region */
    } > boot_region
    .ktext : {
        *(.kentry)          /* kernel's entry code whatever the cause */
        *(.text)            /* code of any object file (except boot) in kernel code region) */
    } > ktext_region
    .kdata : {
        *(.*data*)          /* initialized global variables */
        . = ALIGN(4);       /* move the filling pointer to a word aligned address */
    } > kdata_region
    .kbss : {
        __bss_origin = .;   /* first byte of uninitialized global variables */
        *(.*bss*)           /* uninitialized global variables */
        . = ALIGN(4);       /* move the filling pointer to a word aligned address */
        __bss_end = .;      /* first byte after the bss section */
    } > kdata_region
}
