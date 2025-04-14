/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/dma/soclib-dma.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib dma driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_DMA_H_
#define _SOCLIB_DMA_H_

#include <hal/devices/dma.h>

/**
 * \brief See hal/device/dma.h for the function signature
 * .dma_init   : initialize
 * .dma_memcpy : move data
 */
extern struct dma_ops_s SoclibDMAOps;

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
