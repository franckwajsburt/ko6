/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-26
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platforms/virt/user.ld
  \author   Nolan Bled
  \brief    user application ldscript (SoC dependant)

\*------------------------------------------------------------------------------------------------*/

/* Description of the physical memory map
 * -----------------------------------------------------------------------------
 */

/* Memory segments definition */

__text_origin    = 0x81000000 ; /* first byte address of user code region */
__text_length    = 0x00100000 ;
__data_origin    = 0x81100000 ; /* first byte address of user data region */
__data_length    = 0x00B00000 ;
__data_end       = __data_origin + __data_length ; /* first addr after the user data region */

MEMORY {
    text_region  : ORIGIN = __text_origin,  LENGTH = __text_length
    data_region  : ORIGIN = __data_origin,  LENGTH = __data_length
}

/* Description of the output sections and in which region they are put
 * -----------------------------------------------------------------------------
 */

SECTIONS {
    .text   : {
        *(.text)            /* all others codes */
    } > text_region
    .data : {
        *(.usermem)         /* few addresses in user memory for the user process
        *(.*data*)          /* initialized global variables */
        . = ALIGN(4);       /* move the filling pointer to a word aligned address */
    } > data_region
    .bss : {
        __bss_origin = .;   /* first byte of uninitialized global variables */
        *(.*bss*)           /* uninitialized global variables */
        . = ALIGN(4);       /* move the filling pointer to a word aligned address */
        __bss_end = .;      /* first byte after the bss section */
    }  > data_region
}
