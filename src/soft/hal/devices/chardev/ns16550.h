/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-18
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/tty/ns16550.h
  \author   Nolan Bled
  \brief    NS16550 UART driver
            For a complete description, see http://caro.su/msx/ocm_de1/16550.pdf

\*------------------------------------------------------------------------------------------------*/

#ifndef _NS16550_H_
#define _NS16550_H_

#include <hal/devices/chardev.h>
#include <hal/cpu/irq.h>
#include <kernel/kthread.h>
#include <kernel/klibc.h>

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

/** \brief NS16550 general purpose register map, accessible when LCR.DLAB = 0 */
struct ns16550_general_regs_s {
    unsigned char hr;   //< Transmission/Reception character
    unsigned char ier;  //< Interrupt Enable Register
    unsigned char isr;  //< Interrupt Status Register
    unsigned char fcr;  //< FIFO Control Register
    unsigned char lcr;  //< Line Control Register
    unsigned char mcr;  //< Modem Control Register
    unsigned char lsr;  //< Line Status Register
    unsigned char msr;  //< Modem Status Register
    unsigned char spr;  //< Scratchpad Register
} __attribute__((packed));

/** \brief NS16550 Baudrate control registers, accessible when LCR.DLAB = 1 */
struct ns16550_dlab_regs_s {
    unsigned char dll;          //< Divisor Latch least significant byte
    unsigned char dlm;          //< Divisor Latch most significiant byte
    unsigned char __ignore[5];  //< Padding
    unsigned char psd;          //< Prescaler Division Factor
} __attribute__((packed));

/**
 * \brief   Interrupt Service Routine for NS16550 UART
 *          The ISR push received character into the software fifo
 *          (cdev->fifo)
 * \param   irq the irq linked to this ISR
 * \param   cdev the device linked to this ISR
 */
extern void ns16550_isr(unsigned irq, struct chardev_s *cdev);

extern struct chardev_ops_s NS16550Ops;

#endif