/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/dma/soclib-dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib dma driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_DMA_H_
#define _SOCLIB_DMA_H_

#include <kernel/klibc.h>
#include <hal/devices/dma.h>

struct soclib_dma_regs_s {
    void * src;         ///< dma's destination buffer address
    void * dest;        ///< dma's source buffer address
    int len;            ///< number of bytes to move
    int reset;          ///< IRQ acknowledgement
    int irq_enable;     ///< IRQ mask
    int unused[3];      ///< unused addresses
};

/**
 * \brief See hal/device/dma.h for the function signature
 * .dma_init   : initialize
 * .dma_memcpy : move data
 */
extern struct dma_ops_s SoclibDMAOps;

#endif
