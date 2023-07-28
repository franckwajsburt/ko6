/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/tty/ns16550.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    NS16550 UART driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/chardev/ns16550.h>

static void ns16550_init(struct chardev_s *cdev, unsigned address, unsigned baudrate)
{
    cdev->ops        = &ns16550_ops;
    cdev->address    = address;
    cdev->baudrate   = baudrate;

    volatile struct ns16550_general_regs_s *gregs =
        (struct ns16550_general_regs_s *) address;
    volatile struct ns16550_dlab_regs_s *dregs =
        (struct ns16550_dlab_regs_s *) address;

    // Set baudrate 
    gregs->lcr |= NS16550_ENABLE_DLAB;

    // can't find it elsewhere but xv6 assume a 1843200hz frequency for the uart
    // TODO: handle the PSD register
    unsigned short dl = 1843200 / (16 * baudrate);
    dregs->dll = dl & 0xff;
    dregs->dlm = (dl & 0xff00) >> 8;

    // 8 bits word length, no parity
    gregs->lcr = NS16550_WORD_LENGTH_8;

    // Enable only the interrupt that tells us we receive a character
    // TODO: in the future, we could also manage de THR Empty interrupt
    gregs->ier = NS16550_INT_DATA_READY; 
    
    // Disable hw FIFO
    gregs->fcr = 0;
}

static int ns16550_read(struct chardev_s *cdev, char *buf, unsigned count)
{
    int res = 0;                                        // nb of read char
    int c;                                              // char read

    while (count--) {
        while (chardev_fifo_pull(&cdev->fifo, &c) == FAILURE) {  // wait for a char from the keyboard
            thread_yield();                                 // nothing then we yield the processor
            irq_enable();                                   // get few characters if thread is alone
            irq_disable();                                  // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
}

static int ns16550_write(struct chardev_s *cdev, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    volatile struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) cdev->address;      // access the registers
    
    while (count--) {                                   // while there are chars
        regs->hr = *buf;                                // send the char to TTY
        res++;                                          // nb of written char
        buf++;		                                    // but is the next address in buffer
    }
    return res;
}

struct chardev_ops_s ns16550_ops = {
    .chardev_init = ns16550_init,
    .chardev_read = ns16550_read,
    .chardev_write = ns16550_write
};

void ns16550_isr(unsigned irq, struct chardev_s *cdev)
{
    volatile struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) cdev->address;
    char c = regs->hr;
    chardev_fifo_push(&cdev->fifo, c);
}
