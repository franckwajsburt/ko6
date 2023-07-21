/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     drivers/tty/ns16550.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    NS16550 UART driver

\*------------------------------------------------------------------------------------------------*/

#include <drivers/tty/ns16550.h>

static void ns16550_init(struct tty_s *tty, unsigned address, unsigned baudrate)
{
    tty->ops        = &ns16550_ops;
    tty->address    = address;
    tty->baudrate   = baudrate;
}

static int ns16550_read(struct tty_s *tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of read char
    int c;                                              // char read

    while (count--) {
        while (tty_fifo_pull(&tty->fifo, &c) == FAILURE) {  // wait for a char from the keyboard
            thread_yield();                                 // nothing then we yield the processor
            irq_enable();                                   // get few characters if thread is alone
            irq_disable();                                  // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
}

static int ns16550_write(struct tty_s *tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) tty->address;      // access the registers
    
    while (count--) {                                   // while there are chars
        regs->hr = *buf;                                // send the char to TTY
        res++;                                          // nb of written char
        buf++;		                                    // but is the next address in buffer
    }
    return res;
}

struct tty_ops_s ns16550_ops = {
    .tty_init = ns16550_init,
    .tty_read = ns16550_read,
    .tty_write = ns16550_write
};

void ns16550_isr(unsigned irq, struct tty_s *tty)
{
    struct ns16550_general_regs_s *regs = 
        (struct ns16550_general_regs_s *) tty->address;
    char c = regs->hr;
    tty_fifo_push(&tty->fifo, c);
}
