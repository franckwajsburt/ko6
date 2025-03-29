/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
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
    int write;
    int status;
    int read;
    int unused;
};

/**
 * \brief   ISR for the soclib tty device
            The only interrupt handled is the one raised when a character
            is received by the tty device
 * \param   irq the irq linked to this isr 
 * \param   cdev the device linked to this isr
 * \return  nothing
 */
extern void soclib_tty_isr(unsigned irq, struct chardev_s *cdev);

extern struct chardev_ops_s SoclibTTYOps;

#endif
