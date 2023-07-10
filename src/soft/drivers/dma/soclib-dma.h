/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/dma/soclib-dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib dma driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_DMA_H_
#define _SOCLIB_DMA_H_

/* TODO: Remove dcache_buf_invalidate from klibc and incoporate it into HAL */
#include <hal/dma.h>
#include <klibc.h>

struct soclib_dma_regs_s {
    void * src;         // dma's destination buffer address
    void * dest;        // dma's source buffer address
    int len;            // number of bytes to move
    int reset;          // IRQ acknowledgement
    int irq_enable;     // IRQ mask
    int unused[3];      // unused addresses
};

extern struct dma_ops_s soclib_dma_ops;

#endif