/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/tty/soclib-tty.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Soclib TTY driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_TTY_H_
#define _SOCLIB_TTY_H_

#include <hal/arch/chardev.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>

struct soclib_tty_regs_s {
    int write;
    int status;
    int read;
    int unused;
};

void soclib_tty_isr(unsigned irq, struct chardev_s *cdev);

extern struct chardev_ops_s soclib_tty_ops;

#endif