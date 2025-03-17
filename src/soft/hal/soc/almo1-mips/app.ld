/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-17
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/app.ld
  \author   Franck Wajsburt
  \brief    application ldscript (SoC dependant)

\*------------------------------------------------------------------------------------------------*/

INCLUDE "segment.ld"        /* Memory segments definition */

MEMORY {
    text_region  : ORIGIN = __text_origin,  LENGTH = __text_length
    data_region  : ORIGIN = __data_origin,  LENGTH = __data_length
}

/* Description of the output sections and in which region they are put
 * -----------------------------------------------------------------------------
 */

SECTIONS {
    .text   : {
        *(.text*)           /* all others codes */
    } > text_region
    .data : {
        *(.usermem)         /* few addresses in user memory for the user process
        *(.*data)           /* initialized global variables */
        *(.rodata*)         /* initialized global variables */
    } > data_region
    .bss : {
        . = ALIGN(32);      /* move the filling pointer to a word aligned address */
        __bss_origin = .;   /* first byte of uninitialized global variables */
        *(.*bss)            /* uninitialized global variables */
        *(.bss*)            /* uninitialized global variables */
        . = ALIGN(32);      /* move the filling pointer to a word aligned address */
        __bss_end = .;      /* first byte after the bss section */
    }  > data_region
}
