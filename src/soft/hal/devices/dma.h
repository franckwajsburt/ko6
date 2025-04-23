/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    DMA API

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_DMA_H_
#define _HAL_DMA_H_

struct dma_ops_s;

/** 
 * \brief DMA device specific information 
 */
typedef struct dma_s {
    unsigned base;                  ///< DMA device base address
    unsigned minor;                 ///< device identifier MINOR number
    struct dma_ops_s *ops;          ///< driver-specific operations
} dma_t;

/** 
 * \brief Functions prototypes of the DMA device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct dma_ops_s {
    /**
     * \brief   Generic function that initialize a DMA device
     * \param   dma   the dma device
     * \param   minor minor number is the device instance number
     * \param   base  the base address of the memory-mapped registers
     * \note    almo1-mips : soclib_dma_init
     */
    void (*dma_init)(dma_t *dma, unsigned minor, unsigned base);

    /**
     * \brief   Generic function that copy a buffer from a ocation to another using a DMA device
     * \param   dma   the DMA device
     * \param   dst   destination buffer
     * \param   src   source buffer
     * \param   n     number of bytes to write
     * \return  destination buffer address
     * \note    almo1-mips : soclib_dma_memcpy
    */
    void *(*dma_memcpy)(dma_t *dma, int *dst, int *src, unsigned n);
};

//--------------------------------------------------------------------------------------------------
// dma_alloc()    is used once by soc_init/soc_dma_init to add a new device in the device tree
// dma_count()    returns the number of dmas in the current SoC
// dma_get(minor) returns the dma device structure from its instance number
//--------------------------------------------------------------------------------------------------

#define dma_alloc()    (dma_t *)(dev_alloc(DMA_DEV, sizeof(dma_t))->data)
#define dma_count()    (dev_next_minor(DMA_DEV) - 1)
#define dma_get(minor) (dma_t *)(dev_get(DMA_DEV, minor)->data)

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
