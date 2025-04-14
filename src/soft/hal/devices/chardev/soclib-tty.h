/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/chardev/soclib-tty.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TTY_H_
#define _SOCLIB_TTY_H_

#include <hal/devices/chardev.h>

/**
 * \brief   ISR for the soclib tty device
            The only interrupt handled is the one raised when a char is received by the tty device
 * \param   irq the irq linked to this isr 
 * \param   cdev the device linked to this isr
 * \return  nothing
 */
extern void soclib_tty_isr(unsigned irq, struct chardev_s *cdev);

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
