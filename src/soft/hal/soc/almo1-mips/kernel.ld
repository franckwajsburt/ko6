/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-02-18
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/kernel.ld
  \author   Franck Wajsburt
  \brief    kernel ldscript (SoC dependant)

\*------------------------------------------------------------------------------------------------*/

INCLUDE "segment.ld"

MEMORY {
    boot_region  : ORIGIN = __boot_origin,  LENGTH = __boot_length
    ktext_region : ORIGIN = __ktext_origin, LENGTH = __ktext_length
    kdata_region : ORIGIN = __kdata_origin, LENGTH = __kdata_length
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
        __dtb_address = .;
        *(.dtb)             /* put device tree at the beginning of kernel data */
        *(.*data*)          /* initialized global variables */
        . = ALIGN(4);       /* move the filling pointer to a word aligned address */
    } > kdata_region
    .kbss : {
        __kbss_origin = .;  /* first byte of uninitialized global variables */
        *(.*bss*)           /* uninitialized global variables */
        . = ALIGN(4096);    /* move the filling pointer to a word aligned address */
        __kbss_end = .;     /* first byte after the bss section */
    } > kdata_region
}
