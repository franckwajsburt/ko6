/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/chardev/soclib-tty.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TTY_H_
#define _SOCLIB_TTY_H_

#include <hal/devices/chardev.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>
#include <kernel/klibc.h>

struct soclib_tty_regs_s {
    int write;          ///< output to terminal
    int status;         ///< != 0 if something waiting in read register
    int read;           ///< input from keyboard
    int unused;         ///< no yet used
};

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
