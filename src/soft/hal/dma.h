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
    void (*dma_init)(unsigned address);
    void *(*dma_memcpy)(struct dma_s *dma, int *dst, int *src, unsigned n);
};

struct dma_s {
    unsigned address;
    struct dma_ops_s *ops;
};

extern struct dma_s dma;

#endif