/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-17
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/segs.ld
  \author   Franck Wajsburt
  \brief    Description of the physical memory 
            sed "s/(APPNB)/$APPNB/" segs.ld > segment.ld

\*------------------------------------------------------------------------------------------------*/

__boot_origin   = 0xbfc00000 ;					/* first byte address of boot region */
__boot_length   = 0x00001000 ;					/* boot region size */
__ktext_origin  = 0x80000000 ;					/* first byte address of kernel code region */
__ktext_length  = 0x00100000 ;					
__kdata_origin  = 0x80100000 ;					/* first byte address of kernel data region */
__kdata_length  = 0x0FF00000 ;					
__kdata_end = __kdata_origin + __kdata_length ; /* first addr AFTER kernel data region */
                                                
__text0_origin  = 0x60000000 ;					/* 1st byte address of app0 code region */
__text0_length  = 0x00100000 ;					/* code 1 MB for the app0 : kshell */
__data0_origin  = 0x70000000 ;					/* 1st byte address of app0 data region */
__data0_length  = 0x08000000 ;					/* data 128 MB for the app0 : kshell */
__usermem0      = __data0_origin;				/* user memory descriptor for app0 */
__data0_end = __data0_origin + __data0_length ; /* first addr AFTER the app0 data region */
                                                
__text1_origin  = 0x60100000 ;				 	/* 1st byte address of app1 code region */
__text1_length  = 0x0FF00000 ;                  /* code 255 MB for app1 */
__data1_origin  = 0x78000000 ;                  /* 1st byte address of app1 data region */
__data1_length  = 0x08000000 ;                  /* data 128 MB for app1 */
__usermem1      = __data1_origin;               /* user memory descriptor for app1 */
__data1_end = __data1_origin + __data1_length ;	/* first addr AFTER the app1 data region */

__text_origin   = __text(APPNB)_origin ;		/* 1st byte address of current app code region */
__text_length   = __text(APPNB)_length ;		/* code for the current app */
__data_origin   = __data(APPNB)_origin ;		/* 1st byte address of current app data region */
__data_length   = __data(APPNB)_length ;		/* data for the current app */
__usermem       = __usermem(APPNB);				/* user memory descriptor for the current app */
__data_end      = __data(APPNB)_end;			/* data end for the current app */
