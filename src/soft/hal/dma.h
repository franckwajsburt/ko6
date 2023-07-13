/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    DMA API

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_DMA_H_
#define _HAL_DMA_H_

struct dma_s;

struct dma_ops_s {
    void (*dma_init)(struct dma_s *dma, unsigned address);

    /**
     * \brief Generic function that copy a buffer from a memory location to another using
     *        a DMA device
     * \param   dma the DMA device
     * \param   dst destination buffer
     * \param   src source buffer
     * \param   n   number of bytes to write
     * \return  destination buffer address
    */
    void *(*dma_memcpy)(struct dma_s *dma, int *dst, int *src, unsigned n);
};

struct dma_s {
    unsigned address;       // DMA device address
    struct dma_ops_s *ops;  // driver-specific operations
};
#define dma_alloc() (struct dma_s*) (dev_alloc(DMA_DEV, sizeof(struct dma_s))->data)
#define dma_get(no) (struct dma_s*) (dev_get(DMA_DEV, no)->data)

#endif