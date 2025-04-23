/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-04-23
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     hal/devices/chardev/ns16550.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    NS16550 UART driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/chardev/ns16550.h>

/**
 * \brief   NS16550 UART initialization
 *          The procedure is the following:
 *              * Init the baudrate through de DLAB registers 
 *              * Configure register: 8bits word, no parity check, 1 stop bit
 *              * Enable interrupts
 *              * Disable FIFO
 * \param   cdev   the char device
 * \param   minor  Minor device number (instance number)
 * \param   base   the base NS16550 MMIO address
 * \param   baudrate the baudrate of the UART
 */
static void ns16550_init (chardev_t *cdev, unsigned minor, unsigned base, unsigned baudrate)
{
    cdev->ops       = &NS16550Ops;
    cdev->minor     = minor;
    cdev->base      = base;
    cdev->baudrate  = baudrate;
    
    struct fifo_s *fifo = kmalloc (sizeof(struct fifo_s));
    cdev->driver_data = (void*) fifo;

    volatile struct ns16550_general_regs_s *gregs =
        (struct ns16550_general_regs_s *) base;
    volatile struct ns16550_dlab_regs_s *dregs =
        (struct ns16550_dlab_regs_s *) base;

    /* Set baudrate */
    gregs->lcr |= NS16550_ENABLE_DLAB;

    /**
     * Can't find it elsewhere but xv6 assume a 1843200hz frequency for the uart
     * TODO: handle the PSD register
     */
    unsigned short dl = 1843200 / (16 * baudrate);
    dregs->dll = dl & 0xff;
    dregs->dlm = (dl & 0xff00) >> 8;

    /* 8 bits word length, no parity */
    gregs->lcr = NS16550_WORD_LENGTH_8;

    /**
     * Enable only the interrupt that tells us we receive a character
     * TODO: in the future, we could also manage de THR Empty interrupt
     */
    gregs->ier = NS16550_INT_DATA_READY; 
    
    /* Disable hardware FIFO */
    gregs->fcr = 0;
}

/**
 * \brief   Read a buffer from the NS16550 UART
 * \param   cdev the chardev device corresponding to the NS16550
 * \param   buf the buf to fill
 * \param   count number of bytes to read from the UART
 * \return  number of bytes read
 */
static int ns16550_read (chardev_t *cdev, char *buf, unsigned count)
{
    int res = 0;
    char c;

    struct fifo_s *fifo = (struct fifo_s *) cdev->driver_data;
    while (count--) {
        while (fifo_pull (fifo, &c) == FAILURE) {       // wait for a char from the keyboard
            thread_yield ();                            // nothing then we yield the processor
            irq_enable ();                              // get few characters if thread is alone
            irq_disable ();                             // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
}

/**
 * \brief   Write in the UART
 * \param   cdev the device struct corresponding to the UART
 * \param   buf the buffer to write
 * \param   count number of bytes to write
 * \return  number of bytes written
 */
static int ns16550_write (chardev_t *cdev, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    volatile struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) cdev->base;   // access the registers
    
    while (count--) {                                   // while there are chars
        regs->hr = *buf;                                // send the char to TTY
        res++;                                          // nb of written char
        buf++;		                                    // but is the next base in buffer
    }
    return res;
}

struct chardev_ops_s NS16550Ops = {
    .chardev_init = ns16550_init,
    .chardev_read = ns16550_read,
    .chardev_write = ns16550_write
};

void ns16550_isr (unsigned irq, chardev_t *cdev)
{
    volatile struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) cdev->base;
    
    struct fifo_s *fifo = (struct fifo_s *) cdev->driver_data;
    char c = regs->hr;
    fifo_push (fifo, c);
}
