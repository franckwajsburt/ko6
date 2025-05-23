/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/chardev/soclib-tty.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TTY_H_
#define _SOCLIB_TTY_H_

#include <hal/devices/chardev.h>

/**
 * \brief   ISR for the soclib tty device
 *          The only interrupt handled is the one raised when a char is received by the tty device
 *          This function is used by iunction soc_tty_init() in hal/soc/almo1-mips/soc.c
 *          More specifically by register_interrupt() to fill the InterruptVector[] in kernel/kirq.c
 * \param   irq the irq linked to this isr 
 * \param   cdev the device linked to this isr
 * \return  nothing
 */
extern void soclib_tty_isr (unsigned irq, chardev_t *cdev);

/**
 * \brief See hal/device/chardev.h for the function signature
 * .chardev_init  : initialize
 * .chardev_read  : read the keyboard
 * .chardev_write : write the terminal
 */
extern struct chardev_ops_s SoclibTTYOps;

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
