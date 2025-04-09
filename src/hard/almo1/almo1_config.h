/**
 * \file      config_hard.h
 * \brief     Configuration of Hardware
 * \details  
 * That file contains the hardware configuration : number of cpu/devices and physical address space
 * It is used by the physical prototype simulator and the operating system.
 * It could be possible to change the number of devices or cpus at simulator launch, 
 * but we need to gives the new configuration to the kermel by the mean of file on virtual disk
 * of the prototype. This is not possible for now. When it will be possible, the configuration
 * defined here will be the maximum values eallowed.
 */
#ifndef _CONFIG_HARD_
#define _CONFIG_HARD_

/**
 * ------------------------------------------------------------------------------------------------ 
 * \brief Number Of Resources In The Hardware Prototype
 * ------------------------------------------------------------------------------------------------ 
 * \def CPU_MAX_NR      Number of CPU
 * \def TTY_MAX_NR      Number of Terminal
 * \def DCACHE_LINE_LEN Number od Words in a data cache line
 * \def DCACHE_LINE_WAY Number od ways ( in a data cache line
 * \def DCACHE_LINE_SET Number od Words in a data cache line56
 * \def ICACHE_LINE_LEN 8
 * \def ICACHE_LINE_WAY 4
 * \def ICACHE_LINE_SET 512
 * \def TIMER_MAX_NR    Number of Timers
 * \def DMA_MAX_NR      Number of DMA operator
 * \def FBF_MAX_NR      Number of Frame Buffer 
 * \def BD_MAX_NR       Number of Block Device
 * \def ICU_MAX_NR      Number of ICU
 * \def ICU_MAX_IRQ_NR  Number of IRQ line
 * ------------------------------------------------------------------------------------------------ 
 */

#define CPU_MAX_NR      8
#define TTY_MAX_NR      4
#define DCACHE_LINE_LEN 8
#define DCACHE_LINE_WAY 4
#define DCACHE_LINE_SET 512
#define ICACHE_LINE_LEN 8
#define ICACHE_LINE_WAY 4
#define ICACHE_LINE_SET 512

#define FBF_MAX_NR      1
#define BD_MAX_NR       1
#define TIMER_MAX_NR    CPU_MAX_NR
#define DMA_MAX_NR      1
#define ICU_MAX_NR      CPU_MAX_NR
#define ICU_MAX_IRQ_NR  (TTY_MAX_NR+TIMER_MAX_NR+DMA_MAX_NR+BD_MAX_NR)

/**
 * ------------------------------------------------------------------------------------------------ 
 * \brief Physical Memory Address Space Mapping
 * ------------------------------------------------------------------------------------------------ 
 * \def SEG_RESET_BASE  address boot segment              
 * \def SEG_RESET_SIZE  size of boot segment
 * \def SEG_KERNEL_BASE address kernel's code segment
 * \def SEG_KERNEL_SIZE size of kernel's code segment
 * \def SEG_KDATA_BASE  address kernel's data segment
 * \def SEG_KDATA_SIZE  size of kernel's data segment
 * \def SEG_KUNC_BASE   address uncached data segment
 * \def SEG_KUNC_SIZE   size of uncached data segment
 *
 * \def SEG_DATA_BASE   address user's data segment
 * \def SEG_DATA_SIZE   size of user's data segment
 * \def SEG_CODE_BASE   address user's code segment
 * \def SEG_CODE_SIZE   size of user's code segment
 *
 * \def SEG_TTY_BASE    address device tty's segment (terminals)
 * \def SEG_TTY_SIZE    size of device tty's segment (for all)
 * \def SEG_TTY_SPAN    number of addresses for each tty 
 *
 * \def SEG_TIMER_BASE  address device timer's segment (periodic interrupt requests)
 * \def SEG_TIMER_SIZE  size of device timer's segment (for all timers)
 * \def SEG_TIMER_SPAN  number of addresses for each timer 
 *
 * \def SEG_IOC_BASE    address device ioc's segment (hard disk)
 * \def SEG_IOC_SIZE    size of device ioc's segment (for all)
 * \def SEG_IOC_SPAN    number of addresses for each ioc (for each ioc)
 * 
 * \def SEG_DMA_BASE    address device dma coprocessor's segment (hard memcpy)
 * \def SEG_DMA_SIZE    size of device dma coprocessor's segment (for all)
 * \def SEG_DMA_SPAN    number of addresses for each dma (for each dma)
 * 
 * \def SEG_FBF_BASE    address device fbf's segment (Video Frame Buffer)
 * \def FBF_XSIZE       number of pixels per line
 * \def FBF_YSIZE       number of pixels per column
 * \def SEG_FBF_SIZE    size of device fbf's segment 
 * 
 * \def SEG_ICU_BASE    address device icu's segment (interrupt controler unit)
 * \def SEG_ICU_SIZE    size of device icu's segment (for all icu)
 * \def SEG_ICU_SPAN    number of addresses for each icu
 * ------------------------------------------------------------------------------------------------ 
 */

//---------------------------------------------- KERNEL segments

#define SEG_RESET_BASE  0xbfc00000
#define SEG_RESET_SIZE  0x00001000
#define SEG_KERNEL_BASE 0x80000000
#define SEG_KERNEL_SIZE 0x00100000
#define SEG_KDATA_BASE  0x80100000
#define SEG_KDATA_SIZE  0x0FF00000
#define SEG_KUNC_BASE   0x90000000
#define SEG_KUNC_SIZE   0x00020000

//---------------------------------------------- USER segments

#define SEG_DATA_BASE   0x70000000
#define SEG_DATA_SIZE   0x10000000
#define SEG_CODE_BASE   0x60000000
#define SEG_CODE_SIZE   0x10000000

//---------------------------------------------- Devices segments

#define SEG_TTY_BASE    0xd0200000
#define SEG_TTY_SPAN    0x10
#define SEG_TTY_SIZE    (SEG_TTY_SPAN * TTY_MAX_NR)

#define SEG_DMA_BASE    0xd1200000
#define SEG_DMA_SPAN    0x20
#define SEG_DMA_SIZE    (SEG_DMA_SPAN * DMA_MAX_NR)

#define SEG_ICU_BASE    0xd2200000
#define SEG_ICU_SPAN    0x20
#define SEG_ICU_SIZE    (SEG_ICU_SPAN * ICU_MAX_NR)

#define SEG_TIMER_BASE  0xd3200000
#define SEG_TIMER_SPAN  0x10
#define SEG_TIMER_SIZE  (SEG_TIMER_SPAN * TIMER_MAX_NR)

#define SEG_BD_BASE     0xd5200000
#define SEG_BD_SPAN     0x20
#define SEG_BD_SIZE     (SEG_BD_SPAN * BD_MAX_NR)

#define SEG_FBF_BASE    0x52200000
#define FBF_XSIZE       256
#define FBF_YSIZE       256
#define SEG_FBF_SIZE    0x401000 // 4MB + 4kB (FBF_XSIZE*FBF_YSIZE*2)

//------------------------------------------------------------------------------------------------ 
// Checking the maximum possible number
//------------------------------------------------------------------------------------------------ 

#if CPU_MAX_NR     > 8
#   error  CPU_MAX_NR is too high or too low, see config_hard.h
#endif
#if TTY_MAX_NR     > 8
#   error  TTY_MAX_NR is too high or too low, see config_hard.h
#endif

#if (DCACHE_LINE_LEN != 4) && (DCACHE_LINE_LEN != 4) && (DCACHE_LINE_LEN != 8)
#   error  DCACHE_LINE_LEN must be 2, 4 or 8 
#endif
#if (DCACHE_LINE_WAY != 1) && (DCACHE_LINE_WAY != 2) && (DCACHE_LINE_WAY != 4)
#   error  DCACHE_LINE_WAY must be 1, 2 or 4
#endif
#if (DCACHE_LINE_SET != 2)   && (DCACHE_LINE_SET != 4)   && (DCACHE_LINE_SET != 8) \
 && (DCACHE_LINE_SET != 16)  && (DCACHE_LINE_SET != 32)  && (DCACHE_LINE_SET != 64)\
 && (DCACHE_LINE_SET != 128) && (DCACHE_LINE_SET != 256) && (DCACHE_LINE_SET != 512)
#   error  DCACHE_LINE_SET > must be a power of 2 from 2 to 512
#endif

#if (ICACHE_LINE_LEN != 2) && (ICACHE_LINE_LEN != 4) && (ICACHE_LINE_LEN != 8)
#   error  ICACHE_LINE_LEN must be 2, 4 or 8
#endif
#if (ICACHE_LINE_WAY != 1) && (ICACHE_LINE_WAY != 2) && (ICACHE_LINE_WAY != 4)
#   error  ICACHE_LINE_WAY must be 2, 4 or 4
#endif
#if (ICACHE_LINE_SET != 2)   && (ICACHE_LINE_SET != 4)   && (ICACHE_LINE_SET != 8) \
 && (ICACHE_LINE_SET != 16)  && (ICACHE_LINE_SET != 32)  && (ICACHE_LINE_SET != 64)\
 && (ICACHE_LINE_SET != 128) && (ICACHE_LINE_SET != 256) && (ICACHE_LINE_SET != 512)
#   error  ICACHE_LINE_SET > must be a power of 2 from 2 to 512
#endif

#if FBF_MAX_NR      != 1
#   error  FBF_MAX_NR is too high or too low, see config_hard.h
#endif
#if BD_MAX_NR      != 1
#   error  BD_MAX_NR is too high or too low, see config_hard.h
#endif
#if TIMER_MAX_NR   != CPU_MAX_NR
#   error  TIMER_MAX_NR is too high or too low, see config_hard.h
#endif
#if DMA_MAX_NR     != 1
#   error  DMA_MAX_NR is too high or too low, see config_hard.h
#endif
#if ICU_MAX_NR     != CPU_MAX_NR
#   error  ICU_MAX_NR is too high or too low, see config_hard.h
#endif
#if ICU_MAX_IRQ_NR != (TTY_MAX_NR+TIMER_MAX_NR+DMA_MAX_NR+BD_MAX_NR)
#   error  ICU_MAX_IRQ_NR is too high or too low, see config_hard.h
#endif

#endif
