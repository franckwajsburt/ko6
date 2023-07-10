/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/dma/soclib-dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib dma driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/dma/soclib-dma.h>

void soclib_dma_init(struct dma_s *dma, unsigned address)
{
    dma->address    = address;
    dma->ops        = &soclib_dma_ops;
}

void *soclib_dma_memcpy(struct dma_s *dma, unsigned *dst, unsigned *src, unsigned n)
{
    dcache_buf_invalidate(dst, n);                  // if there are lines of this buffer in the cache, forget them
    volatile struct soclib_dma_regs_s *regs = 
        (struct soclib_dma_regs_s *) dma->address;
    regs->dest = dst;                               // destination address
    regs->src = src;                                // source address
    regs->len = n;                                  // at last number of byte
    while (regs->len) delay(100);
    return dst;
}

struct dma_ops_s soclib_dma_ops = {
    .dma_init = soclib_dma_init,
    .dma_memcpy = soclib_dma_memcpy
};