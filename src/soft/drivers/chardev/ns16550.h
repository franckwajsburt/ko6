/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-18
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/tty/ns16550.h
  \author   Nolan Bled
  \brief    NS16550 UART driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _NS16550_H_
#define _NS16550_H_

#include <hal/tty.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>

struct ns16550_general_regs_s {
    unsigned char hr;
    unsigned char ier;
    unsigned char isr;
    unsigned char fcr;
    unsigned char lcr;
    unsigned char mcr;
    unsigned char lsr;
    unsigned char msr;
    unsigned char spr;
};

struct ns16550_dlab_regs_s {
    unsigned char dll;
    unsigned char dlm;
    unsigned char __ignore[5];
    unsigned char psd;
};

void ns16550_isr(unsigned irq, struct tty_s *tty);

extern struct ns16550_ops_s ns16550_ops;

#endif