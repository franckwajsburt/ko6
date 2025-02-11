/*------------------------------------------------------------------------------------------------*\
   _     ___    __  
  | |__ /'v'\  / /      \date        2023-01-30
  | / /(     )/ _ \     \copyright   2021-2023 sorbonne university 
  |_\_\ x___x \___/                  https://opensource.org/licenses/mit
  
\*------------------------------------------------------------------------------------------------*/

/* Description of the physical memory map
 * -------------------------------------------------------------------------------------------------
 */

/* Device segments definition */

__fb_frame       = 0x52200000 ;           /* first address of frame buffer */ 
__fb_regs_map    = __fb_frame + 0x400000; /* frame maxx ; maxy ; code (420 = YUV420 by default) */ 
__tty_regs_map   = 0xd0200000 ;           /* tty's registers map */
__dma_regs_map   = 0xd1200000 ;           /* dma's registers map */
__bd_regs_map    = 0xd5200000 ;           /* disk's registers map */

/* Memory segments definition */

__boot_origin    = 0xbfc00000 ;     /* first byte address of boot region */
__boot_length    = 0x00001000 ;     /* boot region size */
__trash          = __boot_origin + __boot_length - 4 ;
__data_origin    = 0x7F500000 ;     /* fist byte address of user data region */

MEMORY
{
    boot_region : ORIGIN = __boot_origin, LENGTH = __boot_length
}

/* Description of the output sections and in which region they are put
 * -------------------------------------------------------------------------------------------------
 */

SECTIONS
{
    .boot : {                       /* output section with boot code */
        hcpua.o(.text)              /* add the .text section of hcpua.o */
    } > boot_region                 /* in the boot_region described above */
}
