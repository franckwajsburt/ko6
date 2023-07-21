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

#include <hal/chardev.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>

/* IER Register values*/
#define NS16550_INT_DATA_READY      1
#define NS16550_INT_THR_EMPTY       2
#define NS16550_INT_RECVL_STATUS    4
#define NS16550_INT_MODEM_STATUS    8
#define NS16550_INT_DMA_RX_END      64
#define NS16550_INT_DMA_TX_END      128

/* LCR Register Values*/
#define NS16550_WORD_LENGTH_5   0
#define NS16550_WORD_LENGTH_6   1
#define NS16550_WORD_LENGTH_7   2
#define NS16550_WORD_LENGTH_8   3

#define NS16550_STOPS_BIT_2     4

#define NS16550_PARITY_ODD      8
#define NS16550_PARITY_EVEN     24
#define NS16550_PARITY_FORCED_0 40
#define NS16550_PARITY_FORCED_1 56

#define NS16550_ENABLE_DLAB     128

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

void ns16550_isr(unsigned irq, struct chardev_s *cdev);

extern struct chardev_ops_s ns16550_ops;

#endif