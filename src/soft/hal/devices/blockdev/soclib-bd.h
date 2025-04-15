/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/blockdev/soclib-bd.h
  \author   Franck Wajsburt
  \brief    Soclib block device driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_BD_H_
#define _SOCLIB_BD_H_

#include <hal/devices/blockdev.h>

/**
 * \brief   ISR of the soclib block device
 *          This function is used by iunction soc_bd_init() in hal/soc/almo1-mips/soc.c
 *          More specifically by register_interrupt() to fill the InterruptVector[] in kernel/kirq.c
 * \param   irq irq linked to the ISR 
 * \param   bd  device linked to the ISR
 * \return  nothing
 */
extern void soclib_bd_isr(unsigned irq, struct blockdev_s *bd);

/**
 * \brief See hal/device/blockdev.h for the function signature 
 * .blockdev_init     : initialize
 * .blockdev_read     : read the disk
 * .blockdev_write    : write the disk
 * .blockdev_setevent : define the callback function to be called at  the IRQ event
 */
extern struct blockdev_ops_s SoclibBDOps;

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
