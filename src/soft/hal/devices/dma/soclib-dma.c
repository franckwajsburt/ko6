/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/dma/soclib-dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib dma driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/dma/soclib-dma.h>
#include <kernel/klibc.h>

/**
 * Soclib device registers
 */

struct soclib_dma_regs_s {
    void * src;         ///< dma's destination buffer address
    void * dest;        ///< dma's source buffer address
    int len;            ///< number of bytes to move
    int reset;          ///< IRQ acknowledgement
    int irq_enable;     ///< IRQ mask
    int unused[3];      ///< unused addresses
};

/**
 * \brief   Initialize the Soclib DMA device
 * \param   dma     The dma device 
 * \param   base    The base address of the device
 * \return  nothing
 */
static void soclib_dma_init(struct dma_s *dma, unsigned base)
{
    dma->base    = base;
    dma->ops     = &SoclibDMAOps;
}

/**
 * \brief   Use the DMA device to transfer a buffer 
 * \param   dma the dma device
 * \param   dst destination buffer
 * \param   src source buffer 
 * \param   n number of bytes to copy
 * \return  the pointer to the destination buffer
 */
static void *soclib_dma_memcpy(struct dma_s *dma, int *dst, int *src, unsigned n)
{
    dcache_buf_invalidate(dst, n);              // cached lines of dst buffer are obsolet
    volatile struct soclib_dma_regs_s *regs = 
        (struct soclib_dma_regs_s *) dma->base;
    regs->dest = dst;                           // destination address
    regs->src = src;                            // source address
    regs->len = n;                              // at last number of byte
    while (regs->len) delay(100);
    return dst;
}

struct dma_ops_s SoclibDMAOps = {
    .dma_init = soclib_dma_init,
    .dma_memcpy = soclib_dma_memcpy
};

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
